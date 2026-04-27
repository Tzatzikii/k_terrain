#include "engine_core.hpp"

namespace ec {

#pragma region

bool BaseApp::is_device_suitable( vk::PhysicalDevice device ) {
    VkPhysicalDeviceFeatures supported_features = device.getFeatures();
    ec::QueueFamilyIndices indices = find_queue_families( device );
    
    bool extension_supported = check_device_extension_support( device );

    bool swapchain_adequate = false;

    if( extension_supported ) {
        ec::SwapChainSupportDetails swapchain_support = query_swapchain_support( device ) ;
        swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }

    return 
    indices.is_complete()   && 
    extension_supported     && 
    swapchain_adequate      &&
    supported_features
        .samplerAnisotropy;
}

vk::SampleCountFlagBits get_max_usable_sample_count( vk::PhysicalDevice device ) {
        vk::PhysicalDeviceProperties properties = device.getProperties();

        vk::SampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        if( counts & vk::SampleCountFlagBits::e64 ) { return vk::SampleCountFlagBits::e64; }
        if( counts & vk::SampleCountFlagBits::e32 ) { return vk::SampleCountFlagBits::e32; }
        if( counts & vk::SampleCountFlagBits::e16 ) { return vk::SampleCountFlagBits::e16; }
        if( counts & vk::SampleCountFlagBits::e8  ) { return vk::SampleCountFlagBits::e8 ; }
        if( counts & vk::SampleCountFlagBits::e4  ) { return vk::SampleCountFlagBits::e4 ; }
        if( counts & vk::SampleCountFlagBits::e2  ) { return vk::SampleCountFlagBits::e2 ; }

        return vk::SampleCountFlagBits::e1;
    }

void BaseApp::pick_physical_device() {

    uint32_t device_count = 0;
    
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

    for( const auto& device : devices ) {
        if( is_device_suitable( device ) ) {
            this->physical_device = device;
            msaa_samples = get_max_usable_sample_count( device );
            break;
        }
    }
    
    if( this->physical_device == VK_NULL_HANDLE ) {
        throw std::runtime_error( "failed to find suitable GPU!" );
    }
}

} // namespace ec