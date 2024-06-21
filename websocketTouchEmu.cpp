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

    POINTER_TOUCH_INFO touchInfo = { 0 };
    touchInfo.pointerInfo.pointerType = PT_TOUCH;
    touchInfo.pointerInfo.pointerId = 0;
    touchInfo.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    touchInfo.pointerInfo.ptPixelLocation.x = GetSystemMetrics(SM_CXSCREEN) / 2;
    touchInfo.pointerInfo.ptPixelLocation.y = GetSystemMetrics(SM_CYSCREEN) / 2;
    touchInfo.touchFlags = TOUCH_FLAG_NONE;
    touchInfo.touchMask = TOUCH_MASK_CONTACTAREA;
    touchInfo.rcContact.top = touchInfo.pointerInfo.ptPixelLocation.y - 5;
    touchInfo.rcContact.bottom = touchInfo.pointerInfo.ptPixelLocation.y + 5;
    touchInfo.rcContact.left = touchInfo.pointerInfo.ptPixelLocation.x - 5;
    touchInfo.rcContact.right = touchInfo.pointerInfo.ptPixelLocation.x + 5;
    touchInfo.orientation = 0;
    touchInfo.pressure = 512;

    while (true) {
        InjectTouchInput(1, &touchInfo);

        touchInfo.pointerInfo.pointerFlags = POINTER_FLAG_UP;
        InjectTouchInput(1, &touchInfo);

        Sleep(1000);

        touchInfo.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    }

    return 0;
}