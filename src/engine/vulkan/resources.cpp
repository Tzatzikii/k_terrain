#include "engine_core.hpp"
#include <set>

// i don't know why does it work, but it does!  -kosmx
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ec {

// ----- COMMON -----
#pragma region

std::vector<char> read_file( const std::string& filename ) {
    std::ifstream file( filename, std::ios::ate | std::ios::binary );

    if( !file.is_open() ) {
        throw std::runtime_error( "failed to open file!: " + filename );
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer( file_size );
    file.seekg( 0 );
    file.read( buffer.data(), file_size );
    file.close();

    return buffer;
}

std::vector<const char*> BaseApp::get_required_extensions() {
    uint32_t glfw_extension_count = 0;
    const char ** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

    std::vector<const char*> extensions( glfw_extensions, glfw_extensions + glfw_extension_count );

    if( enable_validation_layers ) {
        extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }

    return extensions;
}

ec::QueueFamilyIndices BaseApp::find_queue_families( vk::PhysicalDevice device ) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queue_families = device.getQueueFamilyProperties();    
    int i = 0;
    for( const auto& queue_family : queue_families ) {
        if( queue_family.queueFlags & vk::QueueFlagBits::eGraphics ) {
            indices.graphics_family = i;
        }
        vk::Bool32 present_support = device.getSurfaceSupportKHR( i, surface );
        if( present_support ) {
            indices.present_family = i;
        }
        if( indices.is_complete() ) {
            break;
        }

        i++;
    }

    return indices;
}

bool BaseApp::check_device_extension_support( vk::PhysicalDevice device ) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );


    std::vector<vk::ExtensionProperties> available_extensions = device.enumerateDeviceExtensionProperties();    
    std::set<std::string> required_extensions( device_extensions.begin(), device_extensions.end() );

    for( const auto& extension : available_extensions) {
        required_extensions.erase( extension.extensionName );
    }
    return required_extensions.empty();
}

uint32_t BaseApp::find_memory_type( uint32_t type_filter, vk::MemoryPropertyFlags properties ) {
    vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

    for( uint32_t i = 0; i < mem_properties.memoryTypeCount; i++ ) {
        if( type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties ) {
            return i;
        }
    }

    throw std::runtime_error( "failed to find suitable memory type!" );
}

vk::Format BaseApp::find_supported_format( const std::vector<vk::Format> & candidates, vk::ImageTiling tiling, 
        vk::FormatFeatureFlags features ) 
{
    for( vk::Format format : candidates ) {
        vk::FormatProperties properties = physical_device.getFormatProperties( format );

        if( tiling == vk::ImageTiling::eLinear && ( properties.linearTilingFeatures & features ) == features ) {
            return format;
        }
        else if( tiling == vk::ImageTiling::eOptimal && ( properties.optimalTilingFeatures & features ) == features ) {
            return format;
        }
    }

    throw std::runtime_error( "failed to find supported format!" );
    
}

vk::CommandBuffer BaseApp::begin_single_time_commands() {
        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = vk::StructureType     ::eCommandBufferAllocateInfo; //VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = vk::CommandBufferLevel::ePrimary; //VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool           = command_pool;
        alloc_info.commandBufferCount    = 1;

        vk::CommandBuffer command_buffer;
        device.allocateCommandBuffers( &alloc_info, &command_buffer );

        vk::CommandBufferBeginInfo begin_info{};
        begin_info.sType = vk::StructureType             ::eCommandBufferBeginInfo; //VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        command_buffer.begin( &begin_info );

        return command_buffer;
}

void BaseApp::end_single_time_commands( vk::CommandBuffer command_buffer ) {
    command_buffer.end();

    vk::SubmitInfo submit_info{};
    submit_info.sType               = vk::StructureType::eSubmitInfo; //VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount  = 1;
    submit_info.pCommandBuffers     = &command_buffer;

    graphics_queue.submit( 1, &submit_info, VK_NULL_HANDLE );
    graphics_queue.waitIdle();
    device.freeCommandBuffers( command_pool, 1, &command_buffer );
}

void BaseApp::copyBuffer( vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size ) {
    vk::CommandBuffer command_buffer = begin_single_time_commands();
    
    vk::BufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    
    command_buffer.copyBuffer( src_buffer, dst_buffer, 1, &copy_region );
    
    end_single_time_commands( command_buffer );
    
}

void BaseApp::copy_buffer_to_image( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height ) {
    vk::CommandBuffer command_buffer = begin_single_time_commands();
    
    vk::BufferImageCopy region{};
    region.bufferOffset         = 0;
    region.bufferRowLength      = 0;
    region.bufferImageHeight    = 0;
    
    region.imageSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel        = 0;
    region.imageSubresource.baseArrayLayer  = 0;
    region.imageSubresource.layerCount      = 1;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{
        width,
        height,
        1
    };

    
    command_buffer.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal, //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands( command_buffer );
}

#pragma endregion

// ----- WINDOW -----
#pragma region

void BaseApp::init_window() {
#ifdef WAYLAND
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    std::cout << "running on wayland" << std::endl;
#elif defined X11
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    std::cout << "running on x11" << std::endl;
#endif
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer( window, this );
    glfwSetFramebufferSizeCallback( window, BaseApp::framebuffer_resize_callback );
}

#pragma endregion

// ----- INSTANCE -----
#pragma region 

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

#pragma endregion

// ----- SURFACE ------
#pragma region
void BaseApp::create_surface() {

    VkSurfaceKHR compat = {};
    if( glfwCreateWindowSurface( instance, window, nullptr, &compat ) != VK_SUCCESS ) {
        throw std::runtime_error( "failed to create window surface!" );
    }
    surface = vk::SurfaceKHR( compat );
}

#pragma endregion

// ----- PHYSICAL DEVICE -----
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

#pragma endregion

// ----- LOGICAL DEVICE -----
#pragma region

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
    
    if( physical_device.createDevice( &create_info, nullptr, &device ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create logical devices!" );
    } 
    
    device.getQueue( indices.graphics_family.value(), 0, &graphics_queue );
    device.getQueue( indices.present_family.value(),  0, &present_queue  );

    VULKAN_HPP_DEFAULT_DISPATCHER.init( device );
    
}

#pragma endregion

// ----- SWAPCHAIN -----
#pragma region

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

#pragma endregion

// ----- IMAGES -----
#pragma region

void BaseApp::transition_image_layout( 
    vk::Image image, 
    vk::Format format, 
    vk::ImageLayout old_layout, 
    vk::ImageLayout new_layout, 
    uint32_t mip_levels 
) {
    vk::CommandBuffer command_buffer = begin_single_time_commands();
    
    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;
    
    
    vk::ImageMemoryBarrier barrier{};
    barrier.sType               = vk::StructureType::eImageMemoryBarrier;//VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout           = old_layout;
    barrier.newLayout           = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    if( 
        old_layout == vk::ImageLayout::eUndefined && //VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == vk::ImageLayout::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
    )
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite; //VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage        = vk::PipelineStageFlagBits::eTopOfPipe; //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage   = vk::PipelineStageFlagBits::eTransfer; //VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( 
        old_layout == vk::ImageLayout::eTransferDstOptimal && //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        new_layout == vk::ImageLayout::eShaderReadOnlyOptimal //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite; //VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead; //VK_ACCESS_SHADER_READ_BIT;
        
        source_stage        = vk::PipelineStageFlagBits::eTransfer; //VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage   = vk::PipelineStageFlagBits::eFragmentShader; //VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument( "unsupported layout transition!" );
    }
    
    command_buffer.pipelineBarrier(
        source_stage, 
        destination_stage,
        {},
        nullptr,
        nullptr, 
        barrier
    );

    end_single_time_commands( command_buffer ); 
}

void BaseApp::create_image( 
    uint32_t                width, 
    uint32_t                height, 
    uint32_t                mip_levels, 
    vk::SampleCountFlagBits sample_count, 
    vk::Format              format, 
    vk::ImageTiling         tiling,
    vk::ImageUsageFlags     usage, 
    vk::MemoryPropertyFlags properties, 
    vk::Image&              image, 
    vk::DeviceMemory&       image_memory 
){
    vk::ImageCreateInfo image_info{};
    image_info.sType         = vk::StructureType ::eImageCreateInfo; //VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = vk::ImageType     ::e2D; //VK_IMAGE_TYPE_2D;
    image_info.extent.width  = static_cast<uint32_t>( width );
    image_info.extent.height = static_cast<uint32_t>( height );
    image_info.extent.depth  = 1;
    image_info.mipLevels     = mip_levels;
    image_info.arrayLayers   = 1;
    image_info.format        = format;
    image_info.tiling        = tiling;
    image_info.initialLayout = vk::ImageLayout   ::eUndefined;  //VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = usage;
    image_info.samples       = sample_count;
    image_info.sharingMode   = vk::SharingMode   ::eExclusive; //VK_SHARING_MODE_EXCLUSIVE;

    if( device.createImage( &image_info, nullptr, &image ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image!" );
    }

    vk::MemoryRequirements mem_requirements;
    device.getImageMemoryRequirements( image, &mem_requirements );

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.sType             = vk::StructureType::eMemoryAllocateInfo; //VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize    = mem_requirements.size;
    alloc_info.memoryTypeIndex   = find_memory_type( mem_requirements.memoryTypeBits, properties );

    if( device.allocateMemory( &alloc_info, nullptr, &image_memory ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image memory!" );
    }

    vkBindImageMemory( device, image, image_memory, 0 );
}

vk::ImageView BaseApp::create_image_view( vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_level ) {
    vk::ImageViewCreateInfo view_info{};
    view_info.sType     = vk::StructureType::eImageViewCreateInfo;//VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image     = image;
    view_info.viewType  = vk::ImageViewType::e2D; //VK_IMAGE_VIEW_TYPE_2D;
    view_info.format    = format;
    view_info.subresourceRange.aspectMask       = aspect_flags;
    view_info.subresourceRange.baseMipLevel     = 0;
    view_info.subresourceRange.levelCount       = mip_level;
    view_info.subresourceRange.baseArrayLayer   = 0;
    view_info.subresourceRange.layerCount       = 1;

    vk::ImageView image_view;
    
    if( device.createImageView( &view_info, nullptr, &image_view ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image view!" );
    }

    return image_view;
}

void BaseApp::create_image_views() {
    swapchain_image_views.resize( swapchain_images.size() );

    for( size_t i = 0; i < swapchain_images.size(); i++ ){
        swapchain_image_views[i] = create_image_view( swapchain_images[i], swapchain_image_format, vk::ImageAspectFlagBits::eColor, 1 );
    }
}

#pragma endregion

// ----- RENDER PASS -----
#pragma region

void BaseApp::create_render_pass() {
    vk::AttachmentDescription color_attachment{};
    color_attachment.format         = swapchain_image_format;
    color_attachment.samples        = this->msaa_samples;
    color_attachment.loadOp         = vk::AttachmentLoadOp  ::eClear;                   //VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = vk::AttachmentStoreOp ::eStore;                   //VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = vk::AttachmentLoadOp  ::eDontCare;                //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp ::eDontCare;                //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = vk::ImageLayout       ::eUndefined;               //VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = vk::ImageLayout       ::eColorAttachmentOptimal;  //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    vk::AttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = vk::ImageLayout::eColorAttachmentOptimal; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vk::AttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format         = swapchain_image_format;
    color_attachment_resolve.samples        = vk::SampleCountFlagBits   ::e1;               //VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp         = vk::AttachmentLoadOp      ::eDontCare;        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp        = vk::AttachmentStoreOp     ::eStore;           //VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp  = vk::AttachmentLoadOp      ::eDontCare;        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = vk::AttachmentStoreOp     ::eDontCare;        //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout  = vk::ImageLayout           ::eUndefined;       //VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout    = vk::ImageLayout           ::ePresentSrcKHR;   //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    vk::AttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout     = vk::ImageLayout::eColorAttachmentOptimal; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vk::AttachmentDescription depth_attachment{};
    depth_attachment.format         = find_depth_format();
    depth_attachment.samples        = this->msaa_samples;
    depth_attachment.loadOp         = vk::AttachmentLoadOp      ::eClear;;                          //VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = vk::AttachmentStoreOp     ::eDontCare;                        //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp  = vk::AttachmentLoadOp      ::eDontCare;                        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.initialLayout  = vk::ImageLayout           ::eUndefined;                       //VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = vk::ImageLayout           ::eDepthStencilAttachmentOptimal;   //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    vk::AttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal; //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics; //VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments     = &color_attachment_resolve_ref;

    std::array<vk::AttachmentDescription, 3> attachments = { 
        color_attachment, depth_attachment, color_attachment_resolve 
    };

    vk::RenderPassCreateInfo render_pass_info{};
    render_pass_info.sType              = vk::StructureType::eRenderPassCreateInfo; // VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount    = static_cast<uint32_t>( attachments.size() );
    render_pass_info.pAttachments       = attachments.data();
    render_pass_info.subpassCount       = 1;
    render_pass_info.pSubpasses         = &subpass;
    
    vk::SubpassDependency dependency{};
    dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass       = 0;
    dependency.srcStageMask     = vk::PipelineStageFlagBits ::eColorAttachmentOutput    |   // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  vk::PipelineStageFlagBits ::eLateFragmentTests;           // VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask    = vk::AccessFlagBits        ::eDepthStencilAttachmentWrite; // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | 
    dependency.dstStageMask     = vk::PipelineStageFlagBits ::eColorAttachmentOutput    |   // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                                  vk::PipelineStageFlagBits ::eEarlyFragmentTests;          // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask    = vk::AccessFlagBits        ::eColorAttachmentWrite     | //VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
                                  vk::AccessFlagBits        ::eDepthStencilAttachmentWrite;// VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    
    if( device.createRenderPass( &render_pass_info, nullptr, &render_pass) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create render pass!" );
    }
}

#pragma endregion

// ----- DESCRIPTOR SET LAYOUT -----
#pragma region

void BaseApp::create_descriptor_set_layout() {
    vk::DescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding              = 0;
    ubo_layout_binding.descriptorType       = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount      = 1;
    ubo_layout_binding.stageFlags           = vk::ShaderStageFlagBits::eVertex; //VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers   = nullptr;

    vk::DescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding            = 1;
    sampler_layout_binding.descriptorCount    = 1;
    sampler_layout_binding.descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags         = vk::ShaderStageFlagBits::eFragment;//VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        ubo_layout_binding, sampler_layout_binding 
    };

    vk::DescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType        = vk::StructureType::eDescriptorSetLayoutCreateInfo; //VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>( bindings.size() );
    layout_info.pBindings    = bindings.data();

    if( device.createDescriptorSetLayout( &layout_info, nullptr, &descriptor_set_layout ) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create descriptor set layout!" );
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType            = vk::StructureType::ePipelineCreateInfoKHR; //VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount   = 1;
    pipelineLayoutInfo.pSetLayouts      = &descriptor_set_layout;
}


#pragma endregion

// ----- GRAPHICS PIPELINE -----
#pragma region

vk::ShaderModule BaseApp::create_shader_module( const std::vector<char>& code ) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo; //VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>( code.data() );

    vk::ShaderModule shaderModule;

    if( device.createShaderModule( &createInfo, nullptr, &shaderModule ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create shader module!" );
    }

    return shaderModule;
}

void BaseApp::create_graphics_pipeline() {

    auto vert_shader_code = read_file( "bin/shaders/vert.spv" );
    auto tesc_shader_code = read_file( "bin/shaders/tesc.spv" );
    auto tese_shader_code = read_file( "bin/shaders/tese.spv" );
    auto frag_shader_code = read_file( "bin/shaders/frag.spv" );

    vk::ShaderModule vert_shader_module = create_shader_module( vert_shader_code );
    vk::ShaderModule tesc_shader_module = create_shader_module( tesc_shader_code );
    vk::ShaderModule tese_shader_module = create_shader_module( tese_shader_code );
    vk::ShaderModule frag_shader_module = create_shader_module( frag_shader_code );


    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eVertex; //VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module   = vert_shader_module;
    vert_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo tesc_shader_stage_info{};
    tesc_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo;
    tesc_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eTessellationControl;
    tesc_shader_stage_info.module   = tesc_shader_module;
    tesc_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo tese_shader_stage_info{};
    tese_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo;
    tese_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eTessellationEvaluation;
    tese_shader_stage_info.module   = tese_shader_module;
    tese_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eFragment; //VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module   = frag_shader_module;
    frag_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vert_shader_stage_info, frag_shader_stage_info
    };

    auto binding_description    = ec::vertex::get_binding_description();
    auto attribute_descriptions = ec::vertex::get_attribute_descriptions();

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType                             = vk::StructureType::ePipelineVertexInputStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount     = 1;
    vertex_input_info.pVertexBindingDescriptions        = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount   = static_cast<uint32_t>( attribute_descriptions.size() );
    vertex_input_info.pVertexAttributeDescriptions      = attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType                    = vk::StructureType     ::ePipelineInputAssemblyStateCreateInfo;  //VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology                 = vk::PrimitiveTopology ::eTriangleList; //VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable   = VK_FALSE;

    vk::Viewport viewport{};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>( swapchain_extent.width );
    viewport.height     = static_cast<float>( swapchain_extent.height );
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapchain_extent;

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport, //VK_DYNAMIC_STATE_VIEWPORT,
        vk::DynamicState::eScissor //VK_DYNAMIC_STATE_SCISSOR
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
    dynamic_state.pDynamicStates    = dynamicStates.data();

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType            = vk::StructureType::ePipelineViewportStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount    = 1;
    viewport_state.scissorCount     = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                    = vk::StructureType     ::ePipelineRasterizationStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable         = VK_FALSE;
    rasterizer.rasterizerDiscardEnable  = VK_FALSE;
    rasterizer.polygonMode              = vk::PolygonMode       ::eLine; //VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                = 1.0f;
    rasterizer.cullMode                 = vk::CullModeFlagBits  ::eBack; //VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                = vk::FrontFace         ::eCounterClockwise; //VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable          = VK_FALSE;
    rasterizer.depthBiasConstantFactor  = 0.0f;
    rasterizer.depthBiasClamp           = 0.0f;
    rasterizer.depthBiasSlopeFactor     = 0.0f;

    vk::PipelineTessellationStateCreateInfo tessellation{};
    tessellation.sType                  = vk::StructureType::ePipelineTessellationStateCreateInfo;
    tessellation.flags                  = {};
    tessellation.patchControlPoints     = 4;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = vk::StructureType::ePipelineMultisampleStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = msaa_samples;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    vk::PipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask       = vk::ColorComponentFlagBits::eR | // VK_COLOR_COMPONENT_R_BIT |
                                                  vk::ColorComponentFlagBits::eG | // VK_COLOR_COMPONENT_G_BIT |
                                                  vk::ColorComponentFlagBits::eB | // VK_COLOR_COMPONENT_B_BIT |
                                                  vk::ColorComponentFlagBits::eR;  // VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable          = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor  = vk::BlendFactor   ::eOne; //VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor  = vk::BlendFactor   ::eZero; //VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp         = vk::BlendOp       ::eAdd; //VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor  = vk::BlendFactor   ::eOne; // VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor  = vk::BlendFactor   ::eOne; //VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp         = vk::BlendOp       ::eAdd; //VK_BLEND_OP_ADD;

    vk::PipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType            = vk::StructureType::ePipelineColorBlendStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable    = VK_FALSE;
    color_blending.logicOp          = vk::LogicOp::eCopy; //VK_LOGIC_OP_COPY;
    color_blending.attachmentCount  = 1;
    color_blending.pAttachments     = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType                 = vk::StructureType::ePipelineDepthStencilStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable       = VK_TRUE;
    depth_stencil.depthWriteEnable      = VK_TRUE;
    depth_stencil.depthCompareOp        = vk::CompareOp::eLess; //VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds        = 0.0f;
    depth_stencil.maxDepthBounds        = 1.0f;
    depth_stencil.stencilTestEnable     = VK_FALSE;
    depth_stencil.front                 = vk::StencilOpState{};
    depth_stencil.back                  = vk::StencilOpState{};

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType                  = vk::StructureType::ePipelineLayoutCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 1;
    pipeline_layout_info.pSetLayouts            = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = nullptr;

    if( device.createPipelineLayout( &pipeline_layout_info, nullptr, &pipeline_layout ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create pipeline layout!" );
    }

    vk::GraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType         = vk::StructureType::eGraphicsPipelineCreateInfo; //VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount    = 2;
    pipeline_info.pStages       = shaderStages;

    pipeline_info.pVertexInputState     = &vertex_input_info;
    pipeline_info.pInputAssemblyState   = &input_assembly;
    pipeline_info.pViewportState        = &viewport_state;
    pipeline_info.pRasterizationState   = &rasterizer;
    pipeline_info.pMultisampleState     = &multisampling;
    pipeline_info.pDepthStencilState    = nullptr;
    pipeline_info.pColorBlendState      = &color_blending;
    pipeline_info.pDepthStencilState    = &depth_stencil;
    pipeline_info.pDynamicState         = &dynamic_state;
    pipeline_info.pTessellationState    = &tessellation;

    pipeline_info.layout        = pipeline_layout;

    pipeline_info.renderPass    = render_pass;
    pipeline_info.subpass       = 0;

    pipeline_info.basePipelineHandle    = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex     = -1; 


    if( device.createGraphicsPipelines( VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create graphics pipeline!" );
    }

    device.destroyShaderModule( frag_shader_module, nullptr );
    device.destroyShaderModule( vert_shader_module, nullptr );

}

#pragma endregion

// ----- COLOR RESOURCES -----
#pragma region

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

#pragma endregion

// ----- DEPTH RESOURCES -----
#pragma region

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

    depth_image_view = create_image_view( depth_image, depth_format, vk::ImageAspectFlagBits::eDepth, 1 );

}

#pragma endregion

// ----- FRAMEBUFFERS -----
#pragma region

void BaseApp::create_framebuffers() {

    swapchain_framebuffers.resize( swapchain_image_views.size() );
        
    for( size_t i = 0; i < swapchain_image_views.size(); i++ ) {
        std::array<vk::ImageView, 3> attachments = {
            color_image_view,
            depth_image_view,
            swapchain_image_views[i],
        };

        vk::FramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType           = vk::StructureType::eFramebufferCreateInfo; //VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>( attachments.size() );
        framebuffer_info.pAttachments    = attachments.data();
        framebuffer_info.width           = swapchain_extent.width;
        framebuffer_info.height          = swapchain_extent.height;
        framebuffer_info.layers          = 1;

        if(  device.createFramebuffer( &framebuffer_info, nullptr, &swapchain_framebuffers[i]) != vk::Result::eSuccess ) {
            throw std::runtime_error( "failed to create framebuffer!" );
        }
    }    

}

#pragma endregion

// ----- COMMAND POOL -----
#pragma region

void BaseApp::create_command_pool() {
    //ec::QueueFamilyIndices q_family_indices = find_queue_families( physical_device );

    vk::CommandPoolCreateInfo pool_info{};
    pool_info.sType = vk::StructureType             ::eCommandPoolCreateInfo; //VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = vk::CommandPoolCreateFlagBits ::eResetCommandBuffer; //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if( device.createCommandPool( &pool_info, nullptr, &command_pool) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create command pool!" );
    }
}

#pragma endregion

// ----- TEXTURES -----
#pragma region

void BaseApp::create_texture_image() {
    int tex_width, tex_height, tex_channels;
    stbi_uc * pixels = stbi_load( TEXTURE_PATH.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha );
    vk::DeviceSize image_size = tex_width * tex_height * 4;

    mip_levels = static_cast<uint32_t>( std::floor( std::log2( std::max( tex_width, tex_height ) ) ) ) + 1;

    if( !pixels ) {
        throw std::runtime_error( "failed to load texture image!" );
    }

    vk::Buffer staging_buffer;
    vk::DeviceMemory staging_buffer_memory;

    create_buffer( 
        image_size, 
        vk::BufferUsageFlagBits     ::eTransferSrc, //VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        vk::MemoryPropertyFlagBits  ::eHostCoherent, //VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        staging_buffer, 
        staging_buffer_memory 
    );
    
    void * data;
    vkMapMemory( device, staging_buffer_memory, 0, image_size, 0, &data );
    std::memcpy( data, pixels, static_cast<size_t>( image_size ) );
    vkUnmapMemory( device, staging_buffer_memory );

    stbi_image_free( pixels );
    
    create_image( 
        tex_width, 
        tex_height, 
        this->mip_levels, 
        vk::SampleCountFlagBits ::e1, //VK_SAMPLE_COUNT_1_BIT, 
        vk::Format              ::eR8G8B8A8Srgb, //VK_FORMAT_R8G8B8A8_SRGB, 
        vk::ImageTiling         ::eOptimal, //VK_IMAGE_TILING_OPTIMAL,

        vk::ImageUsageFlagBits  ::eTransferDst  | // VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
        vk::ImageUsageFlagBits  ::eSampled      | // VK_IMAGE_USAGE_SAMPLED_BIT      | 
        vk::ImageUsageFlagBits  ::eTransferSrc,   // VK_IMAGE_USAGE_TRANSFER_SRC_BIT,

        vk::MemoryPropertyFlagBits::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        texture_image, 
        texture_image_memory 
    );

    transition_image_layout( 
        texture_image, 
        vk::Format      ::eR8G8B8A8Srgb, //VK_FORMAT_R8G8B8A8_SRGB, 
        vk::ImageLayout ::eUndefined,//VK_IMAGE_LAYOUT_UNDEFINED, 
        vk::ImageLayout ::eTransferDstOptimal, //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        this->mip_levels 
    );
    
    //graphics_queue.waitIdle();

    copy_buffer_to_image( 
        staging_buffer, 
        texture_image, 
        static_cast<uint32_t>( tex_width ),
        static_cast<uint32_t>( tex_height ) 
    );

    
    //transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
    //    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->mipLevels );
    
    device.destroyBuffer( staging_buffer, nullptr );
    device.freeMemory( staging_buffer_memory, nullptr );
    
    generate_mipmaps( 
        texture_image, 
        vk::Format::eR8G8B8A8Srgb, //VK_FORMAT_R8G8B8A8_SRGB, 
        tex_width, 
        tex_height, 
        mip_levels 
    );
    
}

void BaseApp::generate_mipmaps( vk::Image image, vk::Format image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels ) {

    vk::FormatProperties format_properties = physical_device.getFormatProperties( image_format );

    if( !( format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear /*VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT*/ ) ) {
        throw std::runtime_error( "texture image format does not support linear blitting!" );
    }

    vk::CommandBuffer command_buffer = begin_single_time_commands();

    vk::ImageMemoryBarrier barrier{};
    barrier.sType = vk::StructureType::eImageMemoryBarrier; //VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.subresourceRange.levelCount     = 1;

    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;

    for( uint32_t i = 1; i < mip_levels; i++ ) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout       = vk::ImageLayout   ::eTransferDstOptimal; //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout       = vk::ImageLayout   ::eTransferSrcOptimal; //VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask   = vk::AccessFlagBits::eTransferWrite; //VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask   = vk::AccessFlagBits::eTransferRead; //VK_ACCESS_TRANSFER_READ_BIT;

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, //VK_PIPELINE_STAGE_TRANSFER_BIT, 
            vk::PipelineStageFlagBits::eTransfer, //VK_PIPELINE_STAGE_TRANSFER_BIT, 
            {},
            nullptr, 
            nullptr,
            barrier 
        );
        
        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = vk::Offset3D{ mip_width, mip_height, 1 };
        blit.srcSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel        = i - 1;
        blit.srcSubresource.baseArrayLayer  = 0;
        blit.srcSubresource.layerCount      = 1;
        blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.dstOffsets[1] = vk::Offset3D{ mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel        = i;
        blit.dstSubresource.baseArrayLayer  = 0;
        blit.dstSubresource.layerCount      = 1; 

        command_buffer.blitImage(
            image, vk::ImageLayout::eTransferSrcOptimal, //VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, vk::ImageLayout::eTransferDstOptimal, //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            vk::Filter::eLinear //VK_FILTER_LINEAR 
        );

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal; //VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead; //VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead; //VK_ACCESS_SHADER_READ_BIT;

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, //VK_PIPELINE_STAGE_TRANSFER_BIT, 
            vk::PipelineStageFlagBits::eFragmentShader, //VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
            {},
            nullptr,
            nullptr,
            barrier 
        );

        if( mip_width  > 1 ) { mip_width  /= 2; }
        if( mip_height > 1 ) { mip_height /= 2; }
    }
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal; //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite; //VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead; //VK_ACCESS_SHADER_READ_BIT;

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, //VK_PIPELINE_STAGE_TRANSFER_BIT, 
        vk::PipelineStageFlagBits::eFragmentShader,//VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
        {},
        nullptr,
        nullptr,
        barrier 
    );

    end_single_time_commands( command_buffer );
}

void BaseApp::create_texture_image_view() {
    texture_image_view = create_image_view( 
        texture_image, 
        vk::Format              ::eR8G8B8A8Srgb, 
        vk::ImageAspectFlagBits ::eColor, 
        this->mip_levels 
    );
}

void BaseApp::create_texture_sampler() {
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.sType          = vk::StructureType     ::eSamplerCreateInfo; //VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter      = vk::Filter            ::eLinear, //VK_FILTER_LINEAR;
    sampler_info.minFilter      = vk::Filter            ::eLinear, //VK_FILTER_LINEAR;
    sampler_info.addressModeU   = vk::SamplerAddressMode::eRepeat,//VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV   = vk::SamplerAddressMode::eRepeat, //VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW   = vk::SamplerAddressMode::eRepeat, //VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;

    vk::PhysicalDeviceProperties properties = physical_device.getProperties();
    sampler_info.maxAnisotropy              = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor                = vk::BorderColor       ::eIntOpaqueBlack; //VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates    = VK_FALSE;
    sampler_info.compareEnable              = VK_FALSE;
    sampler_info.compareOp                  = vk::CompareOp         ::eAlways, //VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode                 = vk::SamplerMipmapMode ::eLinear, //VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias                 = 0.0f;
    sampler_info.minLod                     = 0.0f;
    sampler_info.maxLod                     = VK_LOD_CLAMP_NONE;

    if( device.createSampler( &sampler_info, nullptr, &texture_sampler ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create texture sampler!" );
    }

}

#pragma endregion

// ----- MODELS -----
#pragma region

void BaseApp::load_model() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    if( !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str() ) ) {

        if( !warn.empty() ) {
            std::cout << "tinyobj warning: " << warn << std::endl;
        }
        throw std::runtime_error( err );
    }

    std::unordered_map<vertex, uint32_t> unique_vertices = {};
    
    for( const auto & shape : shapes ) {
        for( const auto & index : shape.mesh.indices ) {
            vertex vertex{};

            
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            vertex.tex_coord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            
            vertex.color = { 1.0f, 1.0f, 1.0f };
            if( unique_vertices.count( vertex ) == 0 ) {
                unique_vertices[vertex] = static_cast<uint32_t>( vertices.size() );
                vertices.push_back( vertex );
            }
            
            indices.push_back( unique_vertices[vertex] );
        }
    }
    
}

#pragma endregion

// ----- DESCRIPTORS -----
#pragma region

void BaseApp::create_descriptor_pool() {
    std::array<vk::DescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type              = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );
    pool_sizes[1].type              = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.sType            = vk::StructureType::eDescriptorPoolCreateInfo; //VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount    = static_cast<uint32_t>( pool_sizes.size() );
    pool_info.pPoolSizes       = pool_sizes.data();
    pool_info.maxSets          = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );

    if( device.createDescriptorPool( &pool_info, nullptr, &descriptor_pool ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create descriptor pool!" );
    }
}

void BaseApp::create_descriptor_sets() {
    std::vector<vk::DescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, descriptor_set_layout );
    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType                = vk::StructureType::eDescriptorSetAllocateInfo; //VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool       = descriptor_pool;
    alloc_info.descriptorSetCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );
    alloc_info.pSetLayouts          = layouts.data();

    descriptor_sets.resize( MAX_FRAMES_IN_FLIGHT );
    if( device.allocateDescriptorSets( &alloc_info, descriptor_sets.data() ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to allocate descriptor sets!" );
    }

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        vk::DescriptorBufferInfo buffer_info{};
        buffer_info.buffer  = uniform_buffers[i];
        buffer_info.offset  = 0;
        buffer_info.range   = sizeof( MVP );

        vk::DescriptorImageInfo image_info{};
        image_info.imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView    = texture_image_view;
        image_info.sampler      = texture_sampler;

        std::array<vk::WriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType              = vk::StructureType::eWriteDescriptorSet; //VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet             = descriptor_sets[i];
        descriptor_writes[0].dstBinding         = 0;
        descriptor_writes[0].dstArrayElement    = 0;
        descriptor_writes[0].descriptorType     = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount    = 1;
        descriptor_writes[0].pBufferInfo        = &buffer_info;

        descriptor_writes[1].sType              = vk::StructureType::eWriteDescriptorSet; //K_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet             = descriptor_sets[i];
        descriptor_writes[1].dstBinding         = 1;
        descriptor_writes[1].dstArrayElement    = 0;
        descriptor_writes[1].descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount    = 1;
        descriptor_writes[1].pImageInfo         = &image_info;
        
        device.updateDescriptorSets( static_cast<uint32_t>( descriptor_sets.size() ), descriptor_writes.data(), 0, nullptr );
        
    }
}

#pragma endregion

// ----- BUFFERS -----
#pragma region

void BaseApp::create_buffer(
    vk::DeviceSize          size, 
    vk::BufferUsageFlags    usage, 
    vk::MemoryPropertyFlags properties,
    vk::Buffer&             buffer, 
    vk::DeviceMemory&       buffer_memory
) {
    vk::BufferCreateInfo buffer_info{};
    buffer_info.sType        = vk::StructureType::eBufferCreateInfo; //VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size         = size;
    buffer_info.usage        = usage;
    buffer_info.sharingMode  = vk::SharingMode::eExclusive; //VK_SHARING_MODE_EXCLUSIVE;

    if( device.createBuffer( &buffer_info, nullptr, &buffer ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create buffer!" );
    }

    vk::MemoryRequirements mem_requirements = device.getBufferMemoryRequirements( buffer );

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.sType            = vk::StructureType::eMemoryAllocateInfo; //VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize   = mem_requirements.size;
    alloc_info.memoryTypeIndex  = find_memory_type( mem_requirements.memoryTypeBits, properties );
    
    if( device.allocateMemory( &alloc_info, nullptr, &buffer_memory ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to allocate buffer memory!" );
    }

    device.bindBufferMemory( buffer, buffer_memory, 0 );
}

void BaseApp::create_vertex_buffer() {
    vk::DeviceSize buffer_size = sizeof( vertices[0] ) * vertices.size();

    vk::Buffer staging_buffer;
    vk::DeviceMemory staging_buffer_memory;
    create_buffer( 
        buffer_size, 
        vk::BufferUsageFlagBits     ::eTransferSrc, //VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        vk::MemoryPropertyFlagBits  ::eHostVisible | //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        vk::MemoryPropertyFlagBits  ::eHostCoherent, //VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, 
        staging_buffer_memory
    );

    void * data;
    vkMapMemory( device, staging_buffer_memory, 0, buffer_size, 0, &data );
    std::memcpy( data, vertices.data(), static_cast<size_t>( buffer_size ));
    vkUnmapMemory( device, staging_buffer_memory );

    create_buffer( buffer_size, 
        vk::BufferUsageFlagBits     ::eTransferDst |//VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
        vk::BufferUsageFlagBits     ::eVertexBuffer,//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        vk::MemoryPropertyFlagBits  ::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        vertex_buffer, 
        vertex_buffer_memory );

    copyBuffer( staging_buffer, vertex_buffer, buffer_size );

    device.destroyBuffer( staging_buffer, nullptr );
    device.freeMemory( staging_buffer_memory, nullptr );
}

void BaseApp::create_index_buffer() {
    vk::DeviceSize buffer_size = sizeof( indices[0] ) * indices.size();

    vk::Buffer staging_buffer;
    vk::DeviceMemory staging_buffer_memory;
    create_buffer( buffer_size, 
        vk::BufferUsageFlagBits     ::eTransferSrc, // VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        vk::MemoryPropertyFlagBits  ::eHostVisible |// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        vk::MemoryPropertyFlagBits  ::eHostCoherent, // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, 
        staging_buffer_memory 
    );

    void * data;
    vkMapMemory( device, staging_buffer_memory, 0, buffer_size, 0, &data );
    std::memcpy( data, indices.data(), static_cast<size_t>( buffer_size ));
    vkUnmapMemory( device, staging_buffer_memory );

    create_buffer( 
        buffer_size, 
        vk::BufferUsageFlagBits     ::eTransferDst | //VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
        vk::BufferUsageFlagBits     ::eIndexBuffer, //VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        vk::MemoryPropertyFlagBits  ::eDeviceLocal, //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        index_buffer, 
        index_buffer_memory 
    );

    copyBuffer( staging_buffer, index_buffer, buffer_size );

    vkDestroyBuffer( device, staging_buffer, nullptr );
    vkFreeMemory( device, staging_buffer_memory, nullptr );
}

void BaseApp::create_uniform_buffers() {
    vk::DeviceSize buffer_size = sizeof( MVP );
    uniform_buffers.resize( MAX_FRAMES_IN_FLIGHT );
    uniform_buffers_memory.resize( MAX_FRAMES_IN_FLIGHT );
    uniform_buffers_mapped.resize( MAX_FRAMES_IN_FLIGHT );
    
    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        create_buffer( 
            buffer_size, 
            vk::BufferUsageFlagBits     ::eUniformBuffer,// VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            vk::MemoryPropertyFlagBits  ::eHostVisible |// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            vk::MemoryPropertyFlagBits  ::eHostCoherent,// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            uniform_buffers[i], 
            uniform_buffers_memory[i] );
        device.mapMemory( uniform_buffers_memory[i], 0, buffer_size, {}, &uniform_buffers_mapped[i] );
    }
}

void BaseApp::update_uniform_buffer( uint32_t currentImage ) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

    MVP mvp{};
    //ubo.model = glm::rotate( glm::mat4(1.0f), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    mvp.model   = glm::identity<glm::mat4>();
    mvp.view    = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f) , glm::vec3( 0.0f, 0.0f, 1.0f ) );
    mvp.proj    = glm::perspective( glm::radians( 45.0f ), swapchain_extent.width / static_cast<float>( swapchain_extent.height ), 0.1f, 10.0f );
    mvp.proj[1][1] *= -1;

    std::memcpy( uniform_buffers_mapped[currentImage], &mvp, sizeof( mvp ));
}

void BaseApp::create_command_buffers() {
        command_buffers.resize( MAX_FRAMES_IN_FLIGHT );

        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.sType             = vk::StructureType::eCommandBufferAllocateInfo; //VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool       = command_pool;
        alloc_info.level             = vk::CommandBufferLevel::ePrimary; //VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = (uint32_t) command_buffers.size();

        if( device.allocateCommandBuffers( &alloc_info, command_buffers.data()) != vk::Result::eSuccess ) {
            throw std::runtime_error( "failed to allocate command buffers!" );
        }
    }

#pragma endregion

// ----- SYNC OBJS ----
#pragma region

void BaseApp::create_sync_objects() {

    image_available_semaphores.resize( MAX_FRAMES_IN_FLIGHT );
    render_finished_semaphores.resize( MAX_FRAMES_IN_FLIGHT );
    in_flight_fences.resize( MAX_FRAMES_IN_FLIGHT );

    vk::SemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = vk::StructureType::eSemaphoreCreateInfo; //VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vk::FenceCreateInfo fence_info{};
    fence_info.sType = vk::StructureType        ::eFenceCreateInfo; //VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = vk::FenceCreateFlagBits  ::eSignaled; //VK_FENCE_CREATE_SIGNALED_BIT;

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        if( device.createSemaphore( &semaphore_info, nullptr, &image_available_semaphores[i] ) != vk::Result::eSuccess ||
            device.createSemaphore( &semaphore_info, nullptr, &render_finished_semaphores[i] ) != vk::Result::eSuccess || 
            device.createFence( &fence_info, nullptr, &in_flight_fences[i] ) != vk::Result::eSuccess ) {
                throw std::runtime_error( "failed to create semaphores!" );
        } 
    }

}

#pragma endregion

// ----- CLEANUP -----
#pragma region

void BaseApp::cleanup_swapchain() {
    device.destroyImageView( color_image_view, nullptr );
    device.destroyImage( color_image, nullptr );
    device.freeMemory( color_image_memory, nullptr );
    device.destroyImageView( depth_image_view, nullptr );
    device.destroyImage( depth_image, nullptr );
    device.freeMemory( depth_image_memory, nullptr );


    for( auto framebuffer : swapchain_framebuffers ) {
        device.destroyFramebuffer( framebuffer, nullptr ); 
    }

    for( auto image_view : swapchain_image_views ) {
        device.destroyImageView( image_view, nullptr );
    }

    device.destroySwapchainKHR( swapchain, nullptr );
}

void BaseApp::cleanup() {
    cleanup_swapchain();

    device.destroySampler( texture_sampler, nullptr );
    device.destroyImageView( texture_image_view, nullptr );

    device.destroyImage( texture_image, nullptr );
    device.freeMemory( texture_image_memory, nullptr );

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        device.destroyBuffer( uniform_buffers[i], nullptr );
        device.freeMemory( uniform_buffers_memory[i], nullptr );
    }

    device.destroyDescriptorPool( descriptor_pool, nullptr );

    device.destroyDescriptorSetLayout( descriptor_set_layout, nullptr );

    device.destroyBuffer( vertex_buffer, nullptr );
    device.freeMemory( vertex_buffer_memory, nullptr );
    device.destroyBuffer( index_buffer, nullptr );
    device.freeMemory( index_buffer_memory, nullptr );

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        device.destroySemaphore( image_available_semaphores[i], nullptr );
        device.destroySemaphore( render_finished_semaphores[i], nullptr );
        device.destroyFence( in_flight_fences[i], nullptr );
    }
    device.destroyCommandPool( command_pool, nullptr );
    
    device.destroyPipeline( graphics_pipeline, nullptr );
    device.destroyPipelineLayout( pipeline_layout, nullptr );
    device.destroyRenderPass( render_pass, nullptr );
    
    if( enable_validation_layers ) {
        instance.destroyDebugUtilsMessengerEXT( debug_messenger ); 
    }
    
    device.destroy( nullptr );
    instance.destroySurfaceKHR( surface, nullptr );
    instance.destroy( nullptr) ;
    

    glfwDestroyWindow( window );

    glfwTerminate();
}

#pragma endregion

} // namespace ec