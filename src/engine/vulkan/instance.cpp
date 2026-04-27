#include "engine_core.hpp"

namespace ec {

void BaseApp::create_instance() {

    if( enable_validation_layers && !BaseApp::check_validation_layer_support() ) {
            throw std::runtime_error( "validation layers requested, but not available!" );
    }
    
    vk::ApplicationInfo appInfo{};
    appInfo.sType               = vk::StructureType::eApplicationInfo; //VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = this->APP_NAME.c_str();
    appInfo.applicationVersion  = VK_MAKE_VERSION(APP_MAJOR_VERSION, APP_MINOR_VERSION, APP_PATCH_VERSION);
    appInfo.pEngineName         = ENGINE_NAME.c_str();
    appInfo.engineVersion       = VK_MAKE_VERSION(ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);
    appInfo.apiVersion          = VK_API_VERSION_1_0;
    
    vk::InstanceCreateInfo createInfo{};
    createInfo.sType            = vk::StructureType::eInstanceCreateInfo; //VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if( enable_validation_layers ) {
        createInfo.enabledLayerCount = static_cast<uint32_t>( validation_layers.size() );
        createInfo.ppEnabledLayerNames = validation_layers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    
    uint32_t glfw_extension_count = 0;
    const char ** glfw_extensions;
    
    glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );
    
    auto extensions = get_required_extensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    //VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    
    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info{};
    
    if( enable_validation_layers ) {
        createInfo.enabledLayerCount = static_cast<uint32_t>( validation_layers.size() );
        createInfo.ppEnabledLayerNames = validation_layers.data();
        
        populate_debug_messenger_create_info( debug_create_info );
        createInfo.pNext = &debug_create_info;
    }
    else {
        createInfo.enabledLayerCount = 0;
        
        createInfo.pNext = nullptr;
    }
    
    if( vk::createInstance( &createInfo, nullptr, &instance ) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create instance" );
    }
    VULKAN_HPP_DEFAULT_DISPATCHER.init( instance );
}

} // namespace ec