#include "engine_core.hpp"

namespace ec {

vk::ShaderModule BaseApp::create_shader_module( const std::vector<char>& code ) {
    vk::ShaderModuleCreateInfo create_info{};
    create_info.sType = vk::StructureType::eShaderModuleCreateInfo; //VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>( code.data() );

    vk::ShaderModule shader_module;

    if( device.createShaderModule( &create_info, nullptr, &shader_module ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create shader module!" );
    }

    return shader_module;
}

void BaseApp::create_graphics_pipeline() {

    auto vert_shader_code = read_file( "bin/shaders/vert.spv" );
    auto tesc_shader_code = read_file( "bin/shaders/tesc.spv" );
    auto tese_shader_code = read_file( "bin/shaders/tese.spv" );
    auto frag_shader_code = read_file( "bin/shaders/frag.spv" );

    vk::ShaderModule vert_shader_module = create_shader_module( vert_shader_code );
    vk::ShaderModule tesc_shader_module = create_shader_module( tesc_shader_code );
    vk::ShaderModule tese_shader_module = create_shader_module( tese_shader_code );
    vk::ShaderModule frag_shader_module = create_shader_module( frag_shader_code );


    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eVertex; //VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module   = vert_shader_module;
    vert_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo tesc_shader_stage_info{};
    tesc_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo;
    tesc_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eTessellationControl;
    tesc_shader_stage_info.module   = tesc_shader_module;
    tesc_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo tese_shader_stage_info{};
    tese_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo;
    tese_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eTessellationEvaluation;
    tese_shader_stage_info.module   = tese_shader_module;
    tese_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType    = vk::StructureType         ::ePipelineShaderStageCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage    = vk::ShaderStageFlagBits   ::eFragment; //VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module   = frag_shader_module;
    frag_shader_stage_info.pName    = "main";

    vk::PipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_info,
        tese_shader_stage_info,
        tesc_shader_stage_info,
        frag_shader_stage_info
    };

    auto vertex_binding_description    = ec::vertex::get_binding_description();
    auto vertex_attribute_descriptions = ec::vertex::get_attribute_descriptions();
    auto instance_binding_description       = ec::Quadtree::LeafInfo::get_binding_description();
    auto instance_attribute_descriptions    = ec::Quadtree::LeafInfo::get_attribute_descriptions();
    constexpr size_t binding_count = 2;
    constexpr size_t attribute_count = vertex_attribute_descriptions.size() + instance_attribute_descriptions.size();

    std::array<vk::VertexInputBindingDescription, binding_count> binding_descriptions = {
        vertex_binding_description, instance_binding_description
    };
    std::array<vk::VertexInputAttributeDescription, attribute_count> attribute_descriptions = {};
    std::copy( vertex_attribute_descriptions.cbegin(), vertex_attribute_descriptions.cend(), attribute_descriptions.begin() );
    std::copy( instance_attribute_descriptions.cbegin(), instance_attribute_descriptions.cend(), attribute_descriptions.begin()+vertex_attribute_descriptions.size() );

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType                             = vk::StructureType::ePipelineVertexInputStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount     = static_cast<uint32_t>( binding_descriptions.size() );
    vertex_input_info.pVertexBindingDescriptions        = binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount   = static_cast<uint32_t>( attribute_descriptions.size() );
    vertex_input_info.pVertexAttributeDescriptions      = attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType                    = vk::StructureType     ::ePipelineInputAssemblyStateCreateInfo;  //VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology                 = vk::PrimitiveTopology ::ePatchList; //VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable   = VK_FALSE;

    vk::Viewport viewport{};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>( swapchain_extent.width );
    viewport.height     = static_cast<float>( swapchain_extent.height );
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapchain_extent;

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport, //VK_DYNAMIC_STATE_VIEWPORT,
        vk::DynamicState::eScissor //VK_DYNAMIC_STATE_SCISSOR
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
    dynamic_state.pDynamicStates    = dynamicStates.data();

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType            = vk::StructureType::ePipelineViewportStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount    = 1;
    viewport_state.scissorCount     = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                    = vk::StructureType     ::ePipelineRasterizationStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable         = VK_FALSE;
    rasterizer.rasterizerDiscardEnable  = VK_FALSE;
    rasterizer.polygonMode              = vk::PolygonMode       ::eLine; //VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                = 1.0f;
    rasterizer.cullMode                 = vk::CullModeFlagBits  ::eBack; //VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                = vk::FrontFace         ::eCounterClockwise; //VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable          = VK_FALSE;
    rasterizer.depthBiasConstantFactor  = 0.0f;
    rasterizer.depthBiasClamp           = 0.0f;
    rasterizer.depthBiasSlopeFactor     = 0.0f;

    vk::PipelineTessellationStateCreateInfo tessellation{};
    tessellation.sType                  = vk::StructureType::ePipelineTessellationStateCreateInfo;
    tessellation.flags                  = {};
    tessellation.patchControlPoints     = 4;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = vk::StructureType::ePipelineMultisampleStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = msaa_samples;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    vk::PipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask       = vk::ColorComponentFlagBits::eR | // VK_COLOR_COMPONENT_R_BIT |
                                                  vk::ColorComponentFlagBits::eG | // VK_COLOR_COMPONENT_G_BIT |
                                                  vk::ColorComponentFlagBits::eB | // VK_COLOR_COMPONENT_B_BIT |
                                                  vk::ColorComponentFlagBits::eR;  // VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable          = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor  = vk::BlendFactor   ::eOne; //VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor  = vk::BlendFactor   ::eZero; //VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp         = vk::BlendOp       ::eAdd; //VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor  = vk::BlendFactor   ::eOne; // VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor  = vk::BlendFactor   ::eOne; //VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp         = vk::BlendOp       ::eAdd; //VK_BLEND_OP_ADD;

    vk::PipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType            = vk::StructureType::ePipelineColorBlendStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable    = VK_FALSE;
    color_blending.logicOp          = vk::LogicOp::eCopy; //VK_LOGIC_OP_COPY;
    color_blending.attachmentCount  = 1;
    color_blending.pAttachments     = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType                 = vk::StructureType::ePipelineDepthStencilStateCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable       = VK_TRUE;
    depth_stencil.depthWriteEnable      = VK_TRUE;
    depth_stencil.depthCompareOp        = vk::CompareOp::eLess; //VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds        = 0.0f;
    depth_stencil.maxDepthBounds        = 1.0f;
    depth_stencil.stencilTestEnable     = VK_FALSE;
    depth_stencil.front                 = vk::StencilOpState{};
    depth_stencil.back                  = vk::StencilOpState{};

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType                  = vk::StructureType::ePipelineLayoutCreateInfo; //VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 1;
    pipeline_layout_info.pSetLayouts            = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = nullptr;

    if( device.createPipelineLayout( &pipeline_layout_info, nullptr, &pipeline_layout ) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create pipeline layout!" );
    }

    vk::GraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType         = vk::StructureType::eGraphicsPipelineCreateInfo; //VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount    = 4;
    pipeline_info.pStages       = shader_stages;

    pipeline_info.pVertexInputState     = &vertex_input_info;
    pipeline_info.pInputAssemblyState   = &input_assembly;
    pipeline_info.pViewportState        = &viewport_state;
    pipeline_info.pRasterizationState   = &rasterizer;
    pipeline_info.pMultisampleState     = &multisampling;
    pipeline_info.pColorBlendState      = &color_blending;
    pipeline_info.pDepthStencilState    = &depth_stencil;
    pipeline_info.pDynamicState         = &dynamic_state;
    pipeline_info.pTessellationState    = &tessellation;

    pipeline_info.layout        = pipeline_layout;

    pipeline_info.renderPass    = render_pass;
    pipeline_info.subpass       = 0;

    pipeline_info.basePipelineHandle    = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex     = -1; 


    if( device.createGraphicsPipelines( VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create graphics pipeline!" );
    }

    device.destroyShaderModule( frag_shader_module, nullptr );
    device.destroyShaderModule( vert_shader_module, nullptr );
    device.destroyShaderModule( tese_shader_module, nullptr );
    device.destroyShaderModule( tesc_shader_module, nullptr );

}

} // namespace ec
