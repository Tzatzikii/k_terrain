#include "engine_core.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace ec {

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


} // namespace ec