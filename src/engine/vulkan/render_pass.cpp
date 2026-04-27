#include "engine_core.hpp"

namespace ec {

void BaseApp::create_render_pass() {
    vk::AttachmentDescription color_attachment{};
    color_attachment.format         = swapchain_image_format;
    color_attachment.samples        = this->msaa_samples;
    color_attachment.loadOp         = vk::AttachmentLoadOp  ::eClear;                   //VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = vk::AttachmentStoreOp ::eStore;                   //VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = vk::AttachmentLoadOp  ::eDontCare;                //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp ::eDontCare;                //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = vk::ImageLayout       ::eUndefined;               //VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = vk::ImageLayout       ::eColorAttachmentOptimal;  //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    vk::AttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = vk::ImageLayout::eColorAttachmentOptimal; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vk::AttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format         = swapchain_image_format;
    color_attachment_resolve.samples        = vk::SampleCountFlagBits   ::e1;               //VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp         = vk::AttachmentLoadOp      ::eDontCare;        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp        = vk::AttachmentStoreOp     ::eStore;           //VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp  = vk::AttachmentLoadOp      ::eDontCare;        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = vk::AttachmentStoreOp     ::eDontCare;        //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout  = vk::ImageLayout           ::eUndefined;       //VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout    = vk::ImageLayout           ::ePresentSrcKHR;   //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    vk::AttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout     = vk::ImageLayout::eColorAttachmentOptimal; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vk::AttachmentDescription depth_attachment{};
    depth_attachment.format         = find_depth_format();
    depth_attachment.samples        = this->msaa_samples;
    depth_attachment.loadOp         = vk::AttachmentLoadOp      ::eClear;;                          //VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = vk::AttachmentStoreOp     ::eDontCare;                        //VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp  = vk::AttachmentLoadOp      ::eDontCare;                        //VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.initialLayout  = vk::ImageLayout           ::eUndefined;                       //VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = vk::ImageLayout           ::eDepthStencilAttachmentOptimal;   //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    vk::AttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal; //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics; //VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments     = &color_attachment_resolve_ref;

    std::array<vk::AttachmentDescription, 3> attachments = { 
        color_attachment, depth_attachment, color_attachment_resolve 
    };

    vk::RenderPassCreateInfo render_pass_info{};
    render_pass_info.sType              = vk::StructureType::eRenderPassCreateInfo; // VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount    = static_cast<uint32_t>( attachments.size() );
    render_pass_info.pAttachments       = attachments.data();
    render_pass_info.subpassCount       = 1;
    render_pass_info.pSubpasses         = &subpass;
    
    vk::SubpassDependency dependency{};
    dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass       = 0;
    dependency.srcStageMask     = vk::PipelineStageFlagBits ::eColorAttachmentOutput    |   // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  vk::PipelineStageFlagBits ::eLateFragmentTests;           // VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask    = vk::AccessFlagBits        ::eDepthStencilAttachmentWrite; // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | 
    dependency.dstStageMask     = vk::PipelineStageFlagBits ::eColorAttachmentOutput    |   // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                                  vk::PipelineStageFlagBits ::eEarlyFragmentTests;          // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask    = vk::AccessFlagBits        ::eColorAttachmentWrite     | //VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
                                  vk::AccessFlagBits        ::eDepthStencilAttachmentWrite;// VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    
    if( device.createRenderPass( &render_pass_info, nullptr, &render_pass) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to create render pass!" );
    }
}


} // namespace ec