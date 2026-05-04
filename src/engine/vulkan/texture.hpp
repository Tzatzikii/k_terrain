#ifndef SRC_ENGINE_VULKAN_TEXTURE_HPP
#define SRC_ENGINE_VULKAN_TEXTURE_HPP

#include <vulkan/vulkan.hpp>
#include <string>
#include "../etc/helpers.hpp"
#include "image.hpp"

namespace ec {


class Texture {


private:

    vk::PhysicalDevice  physical_device;
    vk::Device          device;
    ec::Image2D         image;

    int                 width;
    int                 height;
    int                 channels;
    int                 mip_levels = 1;

    void create_vk( stbi_uc* _pixels, vk::CommandBuffer& _cmd_buffer, vk::Buffer& _staging_buffer );
    stbi_uc* load_image( std::string _path );

public:

    Texture(){}
    Texture( 
        vk::PhysicalDevice& _physical_device, 
        vk::Device&         _device, 
        vk::CommandBuffer&  _cmd_buffer, 
        vk::Buffer&         _staging_buffer, // TEMPORARY!!!!!!!!!!!
        std::string         _path 
    );
    Texture( 
        vk::PhysicalDevice& _physical_device, 
        vk::Device&         _device, 
        vk::CommandBuffer&  _cmd_buffer, 
        vk::Buffer&         _staging_buffer,
        unsigned char*      _pixels, 
        int                 _width, 
        int                 _height 
    );
    vk::ImageView get_view() { return image.get_view(); }
    void create_view( vk::ImageAspectFlags _aspect_flags ) { image.create_view(_aspect_flags); }

    Texture& operator=( const Texture& other ) {
        width = other.width;
        height = other.height;
        channels = other.channels;
        mip_levels = other.mip_levels;
        physical_device = other.physical_device;
        device = other.device;
        image = other.image;
        return *this;
    }


};

} // namespace ec

#endif