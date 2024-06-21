#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <Windows.h>


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

POINTER_TOUCH_INFO toWintouch(drs_touch input) {
    POINTER_TOUCH_INFO retVal{};

    // hardcoded values
    retVal.pointerInfo.pointerType = PT_TOUCH;
    retVal.touchFlags = TOUCH_FLAG_NONE;
    retVal.touchMask = TOUCH_MASK_CONTACTAREA;
    retVal.orientation = 0;
    retVal.pressure = 512;

    //dynamic values
    retVal.pointerInfo.pointerId = input.id; // takes id from drs_touch struct
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



int main()
{
    InitializeTouchInjection(256, TOUCH_FEEDBACK_DEFAULT);

    ix::initNetSystem(); // oh my god this was needed
    int port = 9002;
    std::string host("0.0.0.0");
    ix::WebSocketServer server(port, host);

    server.setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
        // The ConnectionState object contains information about the connection,
        // at this point only the client ip address and the port.
        std::cout << "Remote ip: " << connectionState->getRemoteIp() << std::endl;

        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "New connection" << std::endl;

            // A connection state object is available, and has a default id
            // You can subclass ConnectionState and pass an alternate factory
            // to override it. It is useful if you want to store custom
            // attributes per connection (authenticated bool flag, attributes, etc...)
            std::cout << "id: " << connectionState->getId() << std::endl;

            // The uri the client did connect to.
            std::cout << "Uri: " << msg->openInfo.uri << std::endl;

            std::cout << "Headers:" << std::endl;
            for (auto it : msg->openInfo.headers)
            {
                std::cout << "\t" << it.first << ": " << it.second << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            // For an echo server, we just send back to the client whatever was received by the server
            // All connected clients are available in an std::set. See the broadcast cpp example.
            // Second parameter tells whether we are sending the message in binary or text mode.
            // Here we send it in the same mode as it was received.
            std::cout << "Received: " << msg->str << std::endl;

            webSocket.send(msg->str, msg->binary);
        }
        });

    // Listen for connections
    if (!server.listen().first)
    {
        std::cerr << "Error starting server." << std::endl;
        return 1;
    }

    // Optional: disable per-message deflate to avoid compatibility issues
    server.disablePerMessageDeflate();

    // Start the server
    server.start();

    // Run the server indefinitely
    server.wait();

    return 0;
}