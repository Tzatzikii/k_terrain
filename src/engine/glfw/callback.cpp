#include "callback.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <stdint.h>
#include "../vulkan/engine_core.hpp"


namespace ec {

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ) { 

    BaseApp* baseApp = reinterpret_cast<BaseApp*>( glfwGetWindowUserPointer( window ) );
    baseApp->glfw_key_callback( window, key, scancode, action, mods );

}

void BaseApp::glfw_key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ) {
    int state = glfwGetKey( window, key );

    if( key == GLFW_KEY_ESCAPE && state == GLFW_RELEASE ) {
        auto mode = glfwGetInputMode( window, GLFW_CURSOR );
        if( mode == GLFW_CURSOR_DISABLED ) {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
            cursor_enabled = true;
        }
        else {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
            cursor_enabled = false;
        }

    }


    if( key < 0 || key > keys_pressed.size() - 1 ) {
        return;
    }

    if( state == GLFW_PRESS ) {
        keys_pressed[ key ] = true;
    }
    else if( state == GLFW_RELEASE ) {
        keys_pressed[ key ] = false;
    }

}

void cursor_callback( GLFWwindow* window, double xpos, double ypos ) {
    BaseApp* baseApp = reinterpret_cast<BaseApp*>( glfwGetWindowUserPointer( window ) );
    baseApp->glfw_cursor_callback( window, xpos, ypos );
    
}

void BaseApp::glfw_cursor_callback( GLFWwindow* window, double xpos, double ypos ) {
    if( cursor_enabled ) {
        return;
    }
    cursor_events( xpos, ypos );
}

} // namespace ec