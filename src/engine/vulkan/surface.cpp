#include "engine_core.hpp"

namespace ec {

void BaseApp::create_surface() {

    VkSurfaceKHR compat = {};
    if( glfwCreateWindowSurface( instance, window, nullptr, &compat ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create window surface!" );
    }
    surface = vk::SurfaceKHR( compat );
    
}

} // namespace ec