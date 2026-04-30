#ifndef SRC_ENGINE_GLFW_CALLBACK_HPP
#define SRC_ENGINE_GLFW_CALLBACK_HPP


#include <GLFW/glfw3.h>
#include <array>
#include <stdint.h>

namespace ec {

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
void cursor_callback( GLFWwindow* window, double xpos, double ypos );

}


#endif