#include "image.hpp"

namespace ec {

void Image2D::create( vk::PhysicalDevice& _physical_device, vk::Device& _device, ImageProperties& _properties ) {
    properties = _properties;
    physical_device = _physical_device;
    device = _device;
    vk::ImageCreateInfo image_info{};
    image_info.sType         = vk::StructureType ::eImageCreateInfo; //VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = vk::ImageType     ::e2D; //VK_IMAGE_TYPE_2D;
    image_info.extent.width  = static_cast<uint32_t>( _properties.width );
    image_info.extent.height = static_cast<uint32_t>( _properties.height );
    image_info.extent.depth  = 1;
    image_info.mipLevels     = _properties.mip_levels;
    image_info.arrayLayers   = 1;
    image_info.format        = _properties.format;
    image_info.tiling        = _properties.tiling;
    image_info.initialLayout = vk::ImageLayout   ::eUndefined;  //VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = _properties.usage;
    image_info.samples       = _properties.sample_count;
    image_info.sharingMode   = vk::SharingMode   ::eExclusive; //VK_SHARING_MODE_EXCLUSIVE;

    if( _device.createImage( &image_info, nullptr, &vk_image ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image!" );
    }

    vk::MemoryRequirements mem_requirements;
    _device.getImageMemoryRequirements( vk_image, &mem_requirements );

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.sType             = vk::StructureType::eMemoryAllocateInfo; //VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize    = mem_requirements.size;
    alloc_info.memoryTypeIndex   = ec::find_memory_type( _physical_device, mem_requirements.memoryTypeBits, _properties.property_flags );

    if( _device.allocateMemory( &alloc_info, nullptr, &vk_memory ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image memory!" );
    }

    _device.bindImageMemory( vk_image, vk_memory, 0 );
}

void Image2D::transition_layout( vk::CommandBuffer& _cmd_buffer, vk::ImageLayout _old, vk::ImageLayout _new ) {    
    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;
    
    
    vk::ImageMemoryBarrier barrier{};
    barrier.sType               = vk::StructureType::eImageMemoryBarrier;//VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout           = _old;
    barrier.newLayout           = _new;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = this->vk_image;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor; //VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = this->properties.mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    if( 
        _old == vk::ImageLayout::eUndefined && //VK_IMAGE_LAYOUT_UNDEFINED &&
        _new == vk::ImageLayout::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
    )
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite; //VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage        = vk::PipelineStageFlagBits::eTopOfPipe; //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage   = vk::PipelineStageFlagBits::eTransfer; //VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( 
        _old == vk::ImageLayout::eTransferDstOptimal && //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        _new == vk::ImageLayout::eShaderReadOnlyOptimal //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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
    
    _cmd_buffer.pipelineBarrier(
        source_stage, 
        destination_stage,
        {},
        nullptr,
        nullptr, 
        barrier
    );
}

void Image2D::create_view( vk::ImageAspectFlags _aspect_flags ) {
    vk::ImageViewCreateInfo view_info{};
    view_info.sType     = vk::StructureType::eImageViewCreateInfo;//VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image     = this->vk_image;
    view_info.viewType  = vk::ImageViewType::e2D; //VK_IMAGE_VIEW_TYPE_2D;
    view_info.format    = this->properties.format;
    view_info.subresourceRange.aspectMask       = _aspect_flags;
    view_info.subresourceRange.baseMipLevel     = 0;
    view_info.subresourceRange.levelCount       = properties.mip_levels;
    view_info.subresourceRange.baseArrayLayer   = 0;
    view_info.subresourceRange.layerCount       = 1;
    
    if( device.createImageView( &view_info, nullptr, &view ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create image view!" );
    }
}

void Image2D::copy_from_buffer( vk::CommandBuffer& _cmd_buffer, vk::Buffer& _buffer, uint32_t width, uint32_t height ) {    
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

    _cmd_buffer.copyBufferToImage(
        _buffer,
        this->vk_image,
        vk::ImageLayout::eTransferDstOptimal, //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void Image2D::clean() {
    device.destroyImage( vk_image, nullptr );
    device.destroyImageView( view, nullptr );
    device.freeMemory( vk_memory );

}


}