/* callback.c */


#ifndef CALLBACK_H
#define CALLBACK_H

#include "defines.h"

extern u8 window_resize;

typedef struct {
  f64 mouse_x;
  f64 mouse_y;
  f64 scroll_x;
  f64 scroll_y;
  u8  buttons;
} MouseState;


void framebuffer_resize_callback(GLFWwindow *window, i32 width, i32 height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

#endif
