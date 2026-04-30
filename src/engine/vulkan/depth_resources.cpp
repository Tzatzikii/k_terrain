#include "engine_core.hpp"

namespace ec {

vk::Format BaseApp::find_depth_format() {
    return find_supported_format(
        {
            vk::Format::eD32Sfloat,
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint
        },
        vk::ImageTiling             ::eOptimal,
        vk::FormatFeatureFlagBits   ::eDepthStencilAttachment
    );
}

void BaseApp::create_depth_resources() {
    
    vk::Format depth_format = find_depth_format();
    create_image( 
        swapchain_extent.width, 
        swapchain_extent.height, 
        1, 
        msaa_samples, 
        depth_format,
        vk::ImageTiling             ::eOptimal, //VK_IMAGE_TILING_OPTIMAL, 
        vk::ImageUsageFlagBits      ::eDepthStencilAttachment, //VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        vk::MemoryPropertyFlagBits  ::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        depth_image, 
        depth_image_memory 
    );

    create_image( 
        swapchain_extent.width, 
        swapchain_extent.height, 
        1, 
        msaa_samples, 
        depth_format,
        vk::ImageTiling             ::eOptimal, //VK_IMAGE_TILING_OPTIMAL, 
        vk::ImageUsageFlagBits      ::eDepthStencilAttachment, //VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        vk::MemoryPropertyFlagBits  ::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        shadow_depth_image, 
        shadow_depth_image_memory 
    );

    depth_image_view = create_image_view( 
        depth_image, 
        depth_format, 
        vk::ImageAspectFlagBits::eDepth, 
        1 
    );
    shadow_depth_image_view = create_image_view( 
        shadow_depth_image, 
        depth_format, 
        vk::ImageAspectFlagBits::eDepth,
        1
    );
}

} // namespace ec