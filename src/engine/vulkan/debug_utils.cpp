#include <iostream>
#include "engine_core.hpp"

namespace ec {

VKAPI_ATTR vk::Bool32 VKAPI_CALL BaseApp::debug_callback
( 
    vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void * p_user_data
) 
{
    std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

    return VK_FALSE;
}

bool BaseApp::check_validation_layer_support() {

    uint32_t layer_count;
    vk::enumerateInstanceLayerProperties( &layer_count, nullptr );
    
    std::vector<vk::LayerProperties> available_layers( layer_count );
    vk::enumerateInstanceLayerProperties( &layer_count, available_layers.data() );

    for( const char* layer_name : validation_layers ) {
        bool layer_found = false;

        for( const auto& layer_properties : available_layers ) {
            if( std::strcmp( layer_name, layer_properties.layerName ) == 0 ) {
                layer_found = true;
                break;
            }
        }

        if( !layer_found ) {
            return false;
        }
    }

    return true;

};

void BaseApp::populate_debug_messenger_create_info( vk::DebugUtilsMessengerCreateInfoEXT& create_info) {

    create_info = vk::DebugUtilsMessengerCreateInfoEXT{};

    create_info.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;

    create_info.messageSeverity =   vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
                                    

    create_info.messageType     =   vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral    |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance; 

            
    create_info.pfnUserCallback = BaseApp::debug_callback;
}



void BaseApp::setup_debug_messenger() {
    if( !enable_validation_layers ) return;
    vk::DebugUtilsMessengerCreateInfoEXT create_info;
    VkDebugUtilsMessengerEXT temp;

    populate_debug_messenger_create_info( create_info );

    // auto func = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
        
    // if( func != nullptr ) {
    //     func(instance, &create_info, nullptr, &temp );
    //     debug_messenger = vk::DebugUtilsMessengerEXT( temp );
    // }
    // else {
    //     throw std::runtime_error(" debug utils messenger extension not found! ");
    // }

    instance.createDebugUtilsMessengerEXT( create_info ); 

}  

}; // namespace ec