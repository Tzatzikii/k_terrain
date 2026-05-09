#include "engine_core.hpp"

namespace ec {

void BaseApp::create_logical_device() {
    ec::QueueFamilyIndices indices = find_queue_families( physical_device );
        
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };
    float queue_priority = 1.0f;
    for( uint32_t queueFamily : unique_queue_families ) {
        vk::DeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType             = vk::StructureType::eDeviceQueueCreateInfo; //VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex  = indices.graphics_family.value();
        queue_create_info.queueCount        = 1;
        queue_create_info.pQueuePriorities  = &queue_priority;
        queue_create_infos.push_back( queue_create_info );
    }
    
    vk::PhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.tessellationShader = VK_TRUE;
    device_features.fillModeNonSolid = VK_TRUE;
    
    vk::DeviceCreateInfo create_info{};
    create_info.sType                   = vk::StructureType::eDeviceCreateInfo; //VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_create_infos.data();
    create_info.queueCreateInfoCount    = static_cast<uint32_t>( queue_create_infos.size() );
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = static_cast<uint32_t>( device_extensions.size() );
    create_info.ppEnabledExtensionNames = device_extensions.data();
    
    if( enable_validation_layers ) {
        create_info.enabledLayerCount   = static_cast<uint32_t>( validation_layers.size() );
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else {
        create_info.enabledLayerCount = 0;
    }
    vk::PhysicalDeviceDescriptorIndexingFeatures indexing_features{};
    indexing_features.sType = vk::StructureType::ePhysicalDeviceDescriptorIndexingFeatures;
    indexing_features.shaderSampledImageArrayNonUniformIndexing = vk::True;
    indexing_features.descriptorBindingPartiallyBound = vk::True;

    create_info.pNext = &indexing_features;
    
    if( physical_device.createDevice( &create_info, nullptr, &device ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create logical devices!" );
    } 

    
    device.getQueue( indices.graphics_family.value(), 0, &graphics_queue );
    device.getQueue( indices.present_family.value(),  0, &present_queue  );

    VULKAN_HPP_DEFAULT_DISPATCHER.init( device );
    
}

} // namespace ec