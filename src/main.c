/* main.c - Main hub for Unity-build */

#include "defines.h"
#include "callback.c"
#include "vulkan_main.c"

#include "platform.c"
#include "../lib/volk/volk.c"

void main_loop(void)
{
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(ctx.device);
}

i32 main(i32 argc, char **argv)
{
    platform_init();
    init_window();
    glfwSetWindowUserPointer(ctx.window, &ctx.mouse);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    init_vulkan();
    main_loop();
    cleanup(); 
}
