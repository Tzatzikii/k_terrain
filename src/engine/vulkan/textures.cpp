#include "engine_core.hpp"
#include "texture.hpp"

#include "../../../libs/FastNoise.h"

namespace ec {

void BaseApp::create_texture_image() {
    int tex_width, tex_height, tex_channels;
    stbi_uc * pixels = stbi_load( "res/textures/grass.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha );
    vk::DeviceSize image_size = tex_width * tex_height * 4;
    unsigned char* noise_pixels = new unsigned char[image_size];
    //mip_levels = static_cast<uint32_t>( std::floor( std::log2( std::max( tex_width, tex_height ) ) ) ) + 1;
    mip_levels = 1;
    if( !pixels ) {
        throw std::runtime_error( "failed to load texture image!" );
    }

    chunk_tree->generate_noise_textures( this );
    free(noise_pixels);
    texture = Texture( this, pixels, tex_width, tex_height );

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
    // noise_texture.create_view( 
    //     vk::ImageAspectFlagBits ::eColor
    // );
    texture.create_view( 
        vk::ImageAspectFlagBits ::eColor
    );
    // texture_image_view = create_image_view( 
    //     texture_image, 
    //     vk::Format              ::eR8G8B8A8Srgb, 
    //     vk::ImageAspectFlagBits ::eColor,
    //     this->mip_levels 
    // );
}

void BaseApp::create_texture_sampler() {
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.sType          = vk::StructureType     ::eSamplerCreateInfo; //VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter      = vk::Filter            ::eLinear; //VK_FILTER_LINEAR;
    sampler_info.minFilter      = vk::Filter            ::eLinear; //VK_FILTER_LINEAR;
    sampler_info.addressModeU   = vk::SamplerAddressMode::eClampToEdge;//VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV   = vk::SamplerAddressMode::eClampToEdge; //VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW   = vk::SamplerAddressMode::eClampToEdge; //VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipmapMode     = vk::SamplerMipmapMode ::eNearest;
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
    if( device.createSampler( &sampler_info, nullptr, &noise_sampler ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create texture sampler!" );
    }


}

} // namespace ec