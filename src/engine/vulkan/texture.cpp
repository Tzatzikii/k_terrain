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
    BaseApp*            _current_app,
    std::string         _path 
)
    : current_app(_current_app), physical_device( _current_app->get_physical_device() ), device( _current_app->get_device() )
{
    auto pixels = load_image( _path );
    create_vk( pixels );
    stbi_image_free( pixels ); 
}

Texture::Texture( 
    BaseApp*            _current_app,
    unsigned char*      _pixels, 
    int                 _width, 
    int                 _height 
)
    : current_app(_current_app), physical_device( _current_app->get_physical_device() ), device( _current_app->get_device() ), width(_width), height(_height)
{
    create_vk( _pixels);
}

stbi_uc* Texture::load_image( std::string _path ) {
    stbi_uc * pixels = stbi_load( _path.c_str(), &width, &height, &channels, STBI_rgb_alpha );

    mip_levels = static_cast<uint32_t>( std::floor( std::log2( std::max( width, height ) ) ) ) + 1;

    if( !pixels ) {
        throw std::runtime_error( "failed to load texture image!" );
    }
}

void Texture::create_vk( stbi_uc* _pixels ) {

    vk::CommandBuffer cmd_buffer = current_app->begin_single_time_commands();

    vk::DeviceSize image_size = width * height * 4;
    vk::Buffer staging_buffer;
    vk::DeviceMemory staging_buffer_memory;
    current_app->create_buffer(
        image_size, 
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent,
        staging_buffer,
        staging_buffer_memory
    );

    void * data;
    device.mapMemory( staging_buffer_memory, 0, image_size, {}, &data );
    std::memcpy( data, _pixels, static_cast<size_t>( image_size ) );


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
        cmd_buffer,
        vk::ImageLayout ::eUndefined,//VK_IMAGE_LAYOUT_UNDEFINED, 
        vk::ImageLayout ::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    );

    image.copy_from_buffer(
        cmd_buffer,
        staging_buffer,
        static_cast<uint32_t>( width ),
        static_cast<uint32_t>( height ) 
    );

    image.transition_layout(
        cmd_buffer,
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
    current_app->end_single_time_commands( cmd_buffer );
    device.destroyBuffer(staging_buffer);
    device.freeMemory(staging_buffer_memory);
}


}