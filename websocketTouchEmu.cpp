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