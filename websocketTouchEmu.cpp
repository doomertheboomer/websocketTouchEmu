#include <ixwebsocket/IXWebSocketServer.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <Windows.h>

using json = nlohmann::json;

enum DRS_TOUCH_TYPE {
    DRS_DOWN = 0,
    DRS_UP = 1,
    DRS_MOVE = 2,
};

typedef struct drs_touch {
    int type = DRS_UP;
    int id = 0;
    double x = 0.0;
    double y = 0.0;
    double width = 1;
    double height = 1;
} drs_touch_t;

POINTER_TOUCH_INFO toWintouch(drs_touch input, int fakeId) {
    POINTER_TOUCH_INFO retVal{};

    // hardcoded values
    retVal.pointerInfo.pointerType = PT_TOUCH;
    retVal.touchFlags = TOUCH_FLAG_NONE;
    retVal.touchMask = TOUCH_MASK_CONTACTAREA;
    retVal.orientation = 0;
    retVal.pressure = 512;

    //dynamic values
    retVal.pointerInfo.pointerId = fakeId; // takes id from drs_touch struct
    switch (input.type) { // translates drs input type enum to windows pointer flags
    case DRS_DOWN:
        retVal.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
        break;
    case DRS_MOVE:
        retVal.pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
        break;
    case DRS_UP:
        retVal.pointerInfo.pointerFlags = POINTER_FLAG_UP;
        break;
    }
    retVal.pointerInfo.ptPixelLocation.x = input.x * (double)GetSystemMetrics(SM_CXSCREEN); // 0 = left, 1 = right 
    retVal.pointerInfo.ptPixelLocation.y = input.y * (double)GetSystemMetrics(SM_CYSCREEN); // 0 = top,  1 = bottom
    int sizeX = input.width * (double)GetSystemMetrics(SM_CXSCREEN);
    int sizeY = input.height * (double)GetSystemMetrics(SM_CYSCREEN);
    retVal.rcContact.top = retVal.pointerInfo.ptPixelLocation.y - (sizeX / 2);
    retVal.rcContact.bottom = retVal.pointerInfo.ptPixelLocation.y + (sizeX / 2);
    retVal.rcContact.left = retVal.pointerInfo.ptPixelLocation.x - (sizeY / 2);
    retVal.rcContact.right = retVal.pointerInfo.ptPixelLocation.x + (sizeY / 2);

    return retVal;
}

std::unordered_map<int, drs_touch> touches = {};

std::vector<int> freeIds = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
std::unordered_map<int, int> touchIds = {};

void updateTouches() {
    std::vector<POINTER_TOUCH_INFO> touchInfos = {};
    std::vector<int> deleteTouches = {};
    for (auto& it : touches) {
        int fakeId;
        if (touchIds.count(it.first) < 1) {
            fakeId = freeIds[0]; // using more than 20 touches? your problem
            freeIds.erase(freeIds.begin());
            touchIds.insert_or_assign(it.first, fakeId);
        }
        else {
            fakeId = touchIds[it.first];
        }
        printf("touch id %d = %d\n", it.first, fakeId);

        touchInfos.push_back(toWintouch(it.second, fakeId));
        if (it.second.type == DRS_UP) {

            deleteTouches.push_back(it.first);
        }
        else if (it.second.type == DRS_DOWN) {
            it.second.type = DRS_MOVE;
        }
    }

    // Remove released touch
    for (size_t i = 0; i < deleteTouches.size(); i++) {
        printf("Delete touch %d\n", deleteTouches[i]);
        touches.erase(deleteTouches[i]);
        if (touchIds.count(deleteTouches[i]) > 0) {
            freeIds.push_back(touchIds[deleteTouches[i]]);
            touchIds.erase(deleteTouches[i]);
        }
    }

    if (touchInfos.size() > 0) {
        printf("Theres %d touch infos\n", touchInfos.size());
        InjectTouchInput(touchInfos.size(), touchInfos.data());
    }
}

std::mutex t;
void updateTouch(nlohmann::json params) {
    drs_touch touch{};
    touch.type = params[0];
    touch.id = params[1];
    touch.x = params[2];
    touch.y = params[3];
    touch.width = params[4];
    touch.height = params[5];

    t.lock();
    touches.insert_or_assign(touch.id, touch);
    updateTouches();
    t.unlock();
}

void updater() {

    while (true) {

        t.lock();
        updateTouches();
        t.unlock();

        Sleep(16);
    }
}

int main()
{
    InitializeTouchInjection(256, TOUCH_FEEDBACK_DEFAULT);

    ix::initNetSystem(); // oh my god this was needed
    int port = 9002;
    std::string host("0.0.0.0");
    ix::WebSocketServer server(port, host);

    server.setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Remote ip: " << connectionState->getRemoteIp() << std::endl;
            std::cout << "New connection" << std::endl;
            std::cout << "id: " << connectionState->getId() << std::endl;
            std::cout << "Uri: " << msg->openInfo.uri << std::endl;
            std::cout << "Headers:" << std::endl;
            for (auto it : msg->openInfo.headers)
            {
                std::cout << "\t" << it.first << ": " << it.second << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            json receivedJson = json::parse(msg->str);
            if (receivedJson.contains("params") && receivedJson["params"].is_array() && receivedJson["params"][0].is_array())
            {
                auto params = receivedJson["params"][0];

                if (params.size() == 6)
                {
                    std::cout << "Remote ip: " << connectionState->getRemoteIp() << std::endl;
                    std::cout << "Received: " << msg->str << std::endl;

                    updateTouch(params);
                }
            }
            webSocket.send(msg->str, msg->binary);
        }
        });

    if (!server.listen().first)
    {
        std::cerr << "Error starting server." << std::endl;
        return 1;
    }
    server.disablePerMessageDeflate();
    server.start();

    std::thread upd(updater);
    upd.detach();

    printf("Waiting for server to close\n");
    server.wait();

    return 0;
}