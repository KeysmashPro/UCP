/* main.c - Main hub for Unity-build */

#include "defines.h"
#include "vulkan_core.c"
#include "callback.c"

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
    mouse_state state = {0};
    glfwSetWindowUserPointer(ctx.window, &state);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    init_vulkan();
    main_loop();
    cleanup(); 
}
