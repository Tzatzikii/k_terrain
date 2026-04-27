#include "engine_core.hpp"

namespace ec {

ec::SwapChainSupportDetails BaseApp::query_swapchain_support( vk::PhysicalDevice device ) { 

    SwapChainSupportDetails details;
    
    details.capabilities    = device.getSurfaceCapabilitiesKHR( surface );
    details.formats         = device.getSurfaceFormatsKHR( surface);
    details.present_modes   = device.getSurfacePresentModesKHR( surface );

    return details;

}

vk::SurfaceFormatKHR BaseApp::choose_swap_surface_format( std::vector<vk::SurfaceFormatKHR> &available_formats ) {
    for( const auto& available_format : available_formats ) {
        if( available_format.format     == vk::Format       ::eB8G8R8A8Srgb && // VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear//VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 
        ) {
                return available_format;
            }
    }

    return available_formats[0];
}

vk::PresentModeKHR BaseApp::choose_swap_present_mode( std::vector<vk::PresentModeKHR> present_modes) {
    for( const auto& available_present_mode : present_modes ) {
        if( available_present_mode == vk::PresentModeKHR::eMailbox ) {
            return available_present_mode;
        }
    }
    return vk::PresentModeKHR::eFifo; //VK_PRESENT_MODE_FIFO_KHR;
}

vk::Extent2D BaseApp::choose_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities ) {
    if( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize( window, &width, &height );

        VkExtent2D actualExtent = {
            static_cast<uint32_t>( width ),
            static_cast<uint32_t>( height )
        };

        actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
        actualExtent.height = std::clamp ( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

        return actualExtent;
    }
}

void BaseApp::create_swapchain() {
    ec::SwapChainSupportDetails swapchain_support = query_swapchain_support( physical_device );

    vk::SurfaceFormatKHR surface_format = choose_swap_surface_format( swapchain_support.formats );
    vk::PresentModeKHR present_mode = choose_swap_present_mode( swapchain_support.present_modes );
    vk::Extent2D extent = choose_swap_extent( swapchain_support.capabilities );

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    swapchain_images.resize( image_count );

    if( swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR create_info{};
    create_info.sType            = vk::StructureType::eSwapchainCreateInfoKHR; //VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = surface;
    create_info.minImageCount    = image_count;
    create_info.imageFormat      = surface_format.format;
    create_info.imageColorSpace  = surface_format.colorSpace;
    create_info.imageExtent      = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment; //VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices      = find_queue_families( physical_device );
    uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if( indices.graphics_family != indices.present_family ) {
        create_info.imageSharingMode         = vk::SharingMode::eConcurrent; //VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount    = 2;
        create_info.pQueueFamilyIndices      = queue_family_indices;
    }
    else {
        create_info.imageSharingMode        = vk::SharingMode::eExclusive; //VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount   = 0;
        create_info.pQueueFamilyIndices     = nullptr;
    }

    create_info.preTransform    = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha  = vk::CompositeAlphaFlagBitsKHR::eOpaque; //VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode     = present_mode;
    create_info.clipped         = VK_TRUE;
    create_info.oldSwapchain    = VK_NULL_HANDLE;

    if( device.createSwapchainKHR( &create_info, nullptr, &swapchain) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create swapchain!" );
    }

    device.getSwapchainImagesKHR( swapchain, &image_count, swapchain_images.data() );
    
    swapchain_image_format = surface_format.format;
    swapchain_extent = extent;

}

void BaseApp::recreate_swapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize( window, &width, &height );

    while( width == 0 || height == 0 ) {
        glfwGetFramebufferSize( window, &width, &height );
        glfwWaitEvents();
    }

    vkDeviceWaitIdle( device );

    cleanup_swapchain();

    create_swapchain();
    create_image_views();
    create_color_resources();
    create_depth_resources();
    create_framebuffers();
}

} // namespace ec