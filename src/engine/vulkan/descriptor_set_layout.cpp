#include "engine_core.hpp"

namespace ec {

void BaseApp::create_descriptor_set_layout() {
    vk::DescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding              = 0;
    ubo_layout_binding.descriptorType       = vk::DescriptorType::eUniformBuffer; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount      = 1;
    ubo_layout_binding.stageFlags           = vk::ShaderStageFlagBits::eVertex |
                                              vk::ShaderStageFlagBits::eTessellationEvaluation;//VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers   = nullptr;

    vk::DescriptorSetLayoutBinding noise_sampler_layout_binding{};
    noise_sampler_layout_binding.binding            = 1;
    noise_sampler_layout_binding.descriptorCount    = chunk_tree->get_leaf_count();
    noise_sampler_layout_binding.descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    noise_sampler_layout_binding.pImmutableSamplers = nullptr;
    noise_sampler_layout_binding.stageFlags         = vk::ShaderStageFlagBits::eFragment |
                                                vk::ShaderStageFlagBits::eTessellationEvaluation; //VK_SHADER_STAGE_FRAGMENT_BIT;
                                                
    vk::DescriptorSetLayoutBinding texture_sampler_layout_binding{};
    texture_sampler_layout_binding.binding            = 2;
    texture_sampler_layout_binding.descriptorCount    = 1;
    texture_sampler_layout_binding.descriptorType     = vk::DescriptorType::eCombinedImageSampler; //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_sampler_layout_binding.pImmutableSamplers = nullptr;
    texture_sampler_layout_binding.stageFlags         = vk::ShaderStageFlagBits::eFragment |
                                                 vk::ShaderStageFlagBits::eTessellationEvaluation;

    std::array<vk::DescriptorSetLayoutBinding, 3> bindings = {
        ubo_layout_binding, noise_sampler_layout_binding, texture_sampler_layout_binding
    };

    vk::DescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType        = vk::StructureType::eDescriptorSetLayoutCreateInfo; //VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>( bindings.size() );
    layout_info.pBindings    = bindings.data();

    if( device.createDescriptorSetLayout( &layout_info, nullptr, &descriptor_set_layout ) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create descriptor set layout!" );
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType            = vk::StructureType::ePipelineCreateInfoKHR; //VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount   = 1;
    pipelineLayoutInfo.pSetLayouts      = &descriptor_set_layout;
}

} // namespace ec