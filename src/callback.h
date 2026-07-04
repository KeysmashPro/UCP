/* callback.c */


#ifndef CALLBACK_H
#define CALLBACK_H

#include "defines.h"

typedef struct {
  f32 pos_x;
  f32 pos_y;
  f32 scroll;
  u32 buttons;
} mouse_state;


void framebuffer_resize_callback(GLFWwindow *window, i32 width, i32 height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

#endif
