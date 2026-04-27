#include "engine_core.hpp"

namespace ec {

void BaseApp::create_color_resources() {

    vk::Format color_format = swapchain_image_format;

    create_image( 
        swapchain_extent.width, 
        swapchain_extent.height, 
        1, 
        msaa_samples, 
        color_format,
        vk::ImageTiling             ::eOptimal, //VK_IMAGE_TILING_OPTIMAL, 
        vk::ImageUsageFlagBits      ::eTransientAttachment | //VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | 
        vk::ImageUsageFlagBits      ::eColorAttachment, //VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        vk::MemoryPropertyFlagBits  ::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        color_image, 
        color_image_memory 
    );

    color_image_view = create_image_view( color_image, color_format, vk::ImageAspectFlagBits::eColor /*VK_IMAGE_ASPECT_COLOR_BIT*/, 1 );

}

} // namespace ec