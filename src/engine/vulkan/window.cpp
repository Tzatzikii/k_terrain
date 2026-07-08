#include "engine_core.hpp"
#include "../glfw/callback.hpp"

namespace ec {

    void BaseApp::init_window() {
#ifdef WAYLAND
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    //std::cout << "running on wayland" << std::endl;
#elif defined X11
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    //std::cout << "running on x11" << std::endl;
#endif
    glfwInit();
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* video_mode = glfwGetVideoMode(primary);

    glfwWindowHint( GLFW_POSITION_X, video_mode->width/2 - WIDTH/2 );
    glfwWindowHint( GLFW_POSITION_Y, video_mode->height/2 - HEIGHT/2 );
    
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetWindowUserPointer( window, this );
    glfwSetFramebufferSizeCallback( window, BaseApp::framebuffer_resize_callback );
    glfwSetKeyCallback( window, key_callback );
    glfwSetCursorPosCallback( window, cursor_callback );
}

} // namespace ec