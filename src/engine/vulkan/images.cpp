#include "engine_core.hpp"

namespace ec {

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

} // namespace ec