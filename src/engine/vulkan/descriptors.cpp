#include "engine_core.hpp"

namespace ec {

void BaseApp::create_descriptor_pool() {
    std::array<vk::DescriptorPoolSize, 3> pool_sizes{};
    pool_sizes[0].type              = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );
    pool_sizes[1].type              = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );
    pool_sizes[2].type              = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[2].descriptorCount   = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );

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

        vk::DescriptorImageInfo texture_info{};
        texture_info.imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        texture_info.imageView    = texture.get_view(); //texture_image_view;
        texture_info.sampler      = texture_sampler;

        std::vector<vk::ImageView> noise_views = {};
        //chunk_tree->get_noise_views( noise_views );
        //std::vector<vk::DescriptorImageInfo> noise_images( chunk_tree->get_leaf_count() );
        // for( int i = 0; i < noise_images.size(); i++ ) {
        //     auto& image_info = noise_images[i];
        //     image_info.imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //     image_info.imageView    = noise_views[i]; //texture_image_view;
        //     image_info.sampler      = noise_sampler;
        // }

        // vk::DescriptorImageInfo noise_info{};
        // noise_info.imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // noise_info.imageView    = noise_texture.get_view(); //texture_image_view;
        // noise_info.sampler      = noise_sampler;

        std::array<vk::WriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType              = vk::StructureType::eWriteDescriptorSet; //VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet             = descriptor_sets[i];
        descriptor_writes[0].dstBinding         = 0;
        descriptor_writes[0].dstArrayElement    = 0;
        descriptor_writes[0].descriptorType     = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount    = 1;
        descriptor_writes[0].pBufferInfo        = &buffer_info;

        // descriptor_writes[1].sType              = vk::StructureType::eWriteDescriptorSet; //K_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptor_writes[1].dstSet             = descriptor_sets[i];
        // descriptor_writes[1].dstBinding         = 1;
        // descriptor_writes[1].dstArrayElement    = 0;
        // descriptor_writes[1].descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // descriptor_writes[1].descriptorCount    = noise_images.size();
        // descriptor_writes[1].pImageInfo         = noise_images.data();

        descriptor_writes[1].sType              = vk::StructureType::eWriteDescriptorSet; //K_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet             = descriptor_sets[i];
        descriptor_writes[1].dstBinding         = 2;
        descriptor_writes[1].dstArrayElement    = 0;
        descriptor_writes[1].descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount    = 1;
        descriptor_writes[1].pImageInfo         = &texture_info;
        
        device.updateDescriptorSets( static_cast<uint32_t>( descriptor_writes.size() ), descriptor_writes.data(), 0, nullptr );
        
    }
}

} // namespace ec