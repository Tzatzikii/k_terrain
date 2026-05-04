#include "texture.hpp"
#include "engine_core.hpp"

namespace ec {

//temporary!!!

extern void create_buffer(
    vk::PhysicalDevice      physical_device,
    vk::Device              device,
    vk::DeviceSize          size, 
    vk::BufferUsageFlags    usage, 
    vk::MemoryPropertyFlags properties,
    vk::Buffer&             buffer, 
    vk::DeviceMemory&       buffer_memory
);

Texture::Texture( 
    vk::PhysicalDevice& _physical_device, 
    vk::Device&         _device, 
    vk::CommandBuffer&  _cmd_buffer, 
    vk::Buffer&         _staging_buffer,
    std::string         _path 
)
    : physical_device( _physical_device ), device( _device )
{
    auto pixels = load_image( _path );
    create_vk( pixels, _cmd_buffer, _staging_buffer );
    stbi_image_free( pixels ); 
}

Texture::Texture( 
    vk::PhysicalDevice& _physical_device, 
    vk::Device&         _device, 
    vk::CommandBuffer&  _cmd_buffer, 
    vk::Buffer&         _staging_buffer,
    unsigned char*      _pixels, 
    int                 _width, 
    int                 _height 
)
    : physical_device( _physical_device ), device( _device ), width( _width), height( _height )
{
    create_vk( _pixels, _cmd_buffer, _staging_buffer);
}

stbi_uc* Texture::load_image( std::string _path ) {
    stbi_uc * pixels = stbi_load( _path.c_str(), &width, &height, &channels, STBI_rgb_alpha );

    mip_levels = static_cast<uint32_t>( std::floor( std::log2( std::max( width, height ) ) ) ) + 1;

    if( !pixels ) {
        throw std::runtime_error( "failed to load texture image!" );
    }
}

void Texture::create_vk( stbi_uc* _pixels, vk::CommandBuffer& _cmd_buffer, vk::Buffer& _staging_buffer ) {

    vk::DeviceSize image_size = width * height * 4;
    
    // void * data;
    // vkMapMemory( device, staging_buffer_memory, 0, image_size, 0, &data );
    // std::memcpy( data, _pixels, static_cast<size_t>( image_size ) );
    // vkUnmapMemory( device, staging_buffer_memory );

    ImageProperties properties{
        width, 
        height, 
        this->mip_levels, 
        vk::SampleCountFlagBits ::e1, //VK_SAMPLE_COUNT_1_BIT, 
        vk::Format              ::eR8G8B8A8Srgb, //VK_FORMAT_R8G8B8A8_SRGB, 
        vk::ImageTiling         ::eOptimal, //VK_IMAGE_TILING_OPTIMAL,

        vk::ImageUsageFlagBits  ::eTransferDst  | // VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
        vk::ImageUsageFlagBits  ::eSampled      | // VK_IMAGE_USAGE_SAMPLED_BIT      | 
        vk::ImageUsageFlagBits  ::eTransferSrc,   // VK_IMAGE_USAGE_TRANSFER_SRC_BIT,

        vk::MemoryPropertyFlagBits::eDeviceLocal //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
    };

    image.create( 
        physical_device,
        device,
        properties
    );

    image.transition_layout(
        _cmd_buffer,
        vk::ImageLayout ::eUndefined,//VK_IMAGE_LAYOUT_UNDEFINED, 
        vk::ImageLayout ::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    );

    image.copy_from_buffer(
        _cmd_buffer,
        _staging_buffer,
        static_cast<uint32_t>( width ),
        static_cast<uint32_t>( height ) 
    );

    image.transition_layout(
        _cmd_buffer,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal
    );

    // copy_buffer_to_image( 
    //     staging_buffer, 
    //     texture_image, 
    //     static_cast<uint32_t>( tex_width ),
    //     static_cast<uint32_t>( tex_height ) 
    // );

    
    //transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
    //    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->mipLevels );
    
}


}