#include <iostream>
#include <Windows.h>

int main()
{
    if (!InitializeTouchInjection(256, TOUCH_FEEDBACK_DEFAULT)) {
        std::cerr << "Failed to initialize touch injection: " << GetLastError() << std::endl;
        return 1;
    }

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