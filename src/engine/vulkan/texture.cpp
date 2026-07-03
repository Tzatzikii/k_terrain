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
    int32_t                 _width, 
    int32_t                 _height,
    int32_t                 _count = 1
)
    :   current_app(_current_app), 
        physical_device( _current_app->get_physical_device() ), 
        device( _current_app->get_device() ), 
        width(_width), 
        height(_height),
        count(_count)
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

vk::CommandBuffer Texture::begin_write() {
    vk::CommandBuffer cmd_buffer = current_app->begin_single_time_commands();
    image.transition_layout(
        cmd_buffer,
        vk::ImageLayout ::eShaderReadOnlyOptimal,//VK_IMAGE_LAYOUT_UNDEFINED, 
        vk::ImageLayout ::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    );
    return cmd_buffer;
}

void Texture::end_write( vk::CommandBuffer _cmd_buffer ) {
    image.transition_layout(
        _cmd_buffer,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal
    );
    current_app->end_single_time_commands( _cmd_buffer );
}

void Texture::write( vk::CommandBuffer _cmd_buffer, uint32_t _offset, uint8_t* _pixels, size_t _size ) {
    
    std::memcpy( data + _offset, _pixels, _size ); 
    

    image.copy_from_buffer(
        _cmd_buffer,
        staging_buffer
    );

    
}

void Texture::create_vk( stbi_uc* _pixels ) {

    vk::CommandBuffer cmd_buffer = current_app->begin_single_time_commands();

    vk::DeviceSize image_size = width * height * 4;
    
    current_app->create_buffer(
        image_size * count,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent,
        staging_buffer,
        staging_buffer_memory
    );

    data = device.mapMemory( staging_buffer_memory, 0, image_size );
    std::memcpy( data, _pixels, static_cast<size_t>( image_size ) );

    ec::ImageProperties properties{};
    properties.width        = width;
    properties.height       = height;
    properties.mip_levels   = mip_levels;
    properties.layer_count  = count;
    properties.sample_count = vk::SampleCountFlagBits::e1;
    properties.format       = vk::Format::eR8G8B8A8Srgb;
    properties.tiling       = vk::ImageTiling::eOptimal;
    properties.usage        = vk::ImageUsageFlagBits  ::eTransferDst  |
                              vk::ImageUsageFlagBits  ::eSampled      |
                              vk::ImageUsageFlagBits  ::eTransferSrc,

    properties.memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    image.create( 
        physical_device,
        device,
        properties
    );

    image.transition_layout(
        cmd_buffer,
        vk::ImageLayout ::eUndefined, //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        vk::ImageLayout ::eTransferDstOptimal//VK_IMAGE_LAYOUT_UNDEFINED, 
    );
    // image.transition_layout(
    //     cmd_buffer,
    //     vk::ImageLayout ::eUndefined,//VK_IMAGE_LAYOUT_UNDEFINED, 
    //     vk::ImageLayout ::eTransferDstOptimal //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    // );

    // image.copy_from_buffer(
    //     cmd_buffer,
    //     staging_buffer
    // );

    image.transition_layout(
        cmd_buffer,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal
    );
    current_app->end_single_time_commands( cmd_buffer );
}


}