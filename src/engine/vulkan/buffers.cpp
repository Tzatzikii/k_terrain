#include "engine_core.hpp"
#include <stdint.h>

namespace ec {

extern uint32_t find_memory_type( vk::PhysicalDevice physical_device, uint32_t type_filter, vk::MemoryPropertyFlags properties );


void create_buffer(
    vk::PhysicalDevice      physical_device,
    vk::Device              device,
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
    alloc_info.memoryTypeIndex  = find_memory_type( physical_device, mem_requirements.memoryTypeBits, properties );
    
    if( device.allocateMemory( &alloc_info, nullptr, &buffer_memory ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to allocate buffer memory!" );
    }

    device.bindBufferMemory( buffer, buffer_memory, 0 );
}

// Temporary wrapper
void BaseApp::create_buffer(
    vk::DeviceSize          size, 
    vk::BufferUsageFlags    usage, 
    vk::MemoryPropertyFlags properties,
    vk::Buffer&             buffer, 
    vk::DeviceMemory&       buffer_memory
) {
    ec::create_buffer( physical_device, device, size, usage, properties, buffer, buffer_memory );
}

void BaseApp::create_vertex_buffer() {
    vk::DeviceSize buffer_size = sizeof( vertices[0] ) * vertices.size();
    std::cout << vertices.size() << std::endl;
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

    copy_buffer( staging_buffer, vertex_buffer, buffer_size );

    device.destroyBuffer( staging_buffer, nullptr );
    device.freeMemory( staging_buffer_memory, nullptr );
}

void BaseApp::recreate_vertex_buffer() {
    device.destroyBuffer( vertex_buffer );
    device.freeMemory( vertex_buffer_memory );
    create_vertex_buffer();
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

    copy_buffer( staging_buffer, index_buffer, buffer_size );

    vkDestroyBuffer( device, staging_buffer, nullptr );
    vkFreeMemory( device, staging_buffer_memory, nullptr );
}

void BaseApp::create_instance_buffer() {
    static void* data;
    if( instance_buffer == VK_NULL_HANDLE ) {
        // This would mean that we are standing in the middle of a
        // chunk grid the size of INT32_MAX.
        // Since the chunks get exponentially larger, in 1D the amount of chunks is
        // log2( INT32_MAX - INT32_MIN ) = log2( 2^64 ) = 64.
        // 4096 is just 64 squared since it's a 2D grid.
        const uint32_t very_max_chunk_count = 4096; 
        vk::DeviceSize size = very_max_chunk_count * sizeof( ec::Quadtree::LeafInfo );
        create_buffer( 
            size,
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostCoherent |
            vk::MemoryPropertyFlagBits::eHostVisible,
            instance_buffer,
            instance_buffer_memory
        );
        data = device.mapMemory( instance_buffer_memory, 0, size );
    }

    std::memcpy( data, instances.data(), instances.size() * sizeof( ec::Quadtree::LeafInfo ) );
    
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
    mvp.view    = glm::lookAt( camera.get_pos(), camera.get_pos()+camera.get_dir(), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    mvp.proj    = glm::perspective( glm::radians( 45.0f ), swapchain_extent.width / static_cast<float>( swapchain_extent.height ), 0.1f, std::pow(2.0f, 20.0f) );
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


void BaseApp::create_framebuffers() {

    swapchain_framebuffers.resize( swapchain_image_views.size() );
        
    for( size_t i = 0; i < swapchain_image_views.size(); i++ ) {
        std::array<vk::ImageView, 3> attachments = {
            color_image_view,
            depth_image_view,
            //shadow_depth_image_view,
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

} // namespace ec