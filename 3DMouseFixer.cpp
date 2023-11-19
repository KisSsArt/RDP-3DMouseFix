#include <iostream>
#include <cmath>
#include <algorithm>

#include "library/interception.h"
#include "samples/utils.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

enum ScanCode
{
    SCANCODE_ESC = 0x01,
    VK_P = 25
};

int main()
{
    bool in_interception = false;
    int oldX = 0;
    int oldY = 0;
    const float sensitivity = 10;

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        bool sended = false;

        if (interception_is_keyboard(device))
        {
            InterceptionKeyStroke& kstroke = *(InterceptionKeyStroke*)&stroke;

            interception_send(context, device, &stroke, 1);
            sended = true;

            if (kstroke.code == VK_P)
            {
                in_interception = !in_interception;
                in_interception ? std::cout << "interception: true" << std::endl : std::cout << "interception: false" << std::endl;
            }
            
            if (kstroke.code == SCANCODE_ESC)
                break;
        }

        if (!in_interception)
        {
            if (interception_is_mouse(device))
                interception_send(context, device, &stroke, 1);

            continue;
        }

        if (interception_is_mouse(device))
        {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

            if (!oldX || !oldY)
            {
                oldX = mstroke.x;
                oldY = mstroke.y;
            }

            //if (!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE))
            {
                int x = mstroke.x;
                int y = mstroke.y;

                mstroke.x = std::clamp(-(oldX - mstroke.x), -1, 1) * sensitivity;
                mstroke.y = std::clamp(-(oldY - mstroke.y), -1, 1) * sensitivity;

                oldX = x;
                oldY = y;

                std::cout << "old mouse " << oldX << ":" << oldY << " new " << mstroke.x << ":" << mstroke.y << std::endl;
            }

            interception_send(context, device, &stroke, 1);
        }
    }

    interception_destroy_context(context);

    return 0;
}