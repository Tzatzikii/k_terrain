#ifndef SRC_ENGINE_HELPERS_HPP
#define SRC_ENGINE_HELPERS_HPP

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <fstream>
#include "lib_includes.hpp"


namespace ec {
    
struct MVP {

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

};

struct QueueFamilyIndices {

    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() { 
        return graphics_family.has_value() && present_family.has_value();
    }

};

struct SwapChainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;

};

}; // namespace ec


#endif // SRC_ENGINE_HELPERS_HPP