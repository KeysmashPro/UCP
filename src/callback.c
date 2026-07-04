/* callback.c */

#include "callback.h"
#include "vulkan_header.h"
#include "defines.h"

void framebuffer_resize_callback(GLFWwindow* window, i32 width, i32 height) {
    ctx.resize_request = 1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
      switch(key) {
        case GLFW_KEY_Q:  glfwSetWindowShouldClose(window, GLFW_TRUE); break;
      }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(window);
    switch(button) {
      case GLFW_MOUSE_BUTTON_LEFT:   state->buttons = (action == GLFW_PRESS) ? state->buttons | (1 << 0) : state->buttons & ~(1 << 0); break;
      case GLFW_MOUSE_BUTTON_RIGHT:  state->buttons = (action == GLFW_PRESS) ? state->buttons | (1 << 1) : state->buttons & ~(1 << 1); break;
      case GLFW_MOUSE_BUTTON_MIDDLE: state->buttons = (action == GLFW_PRESS) ? state->buttons | (1 << 2) : state->buttons & ~(1 << 2); break;
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(window);
    state->pos_x = xpos;
    state->pos_y = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(window);
    state->scroll = yoffset;
}

