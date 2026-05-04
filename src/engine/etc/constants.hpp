#ifndef SRC_ENGINE_ETC_CONSTANTS_HPP
#define SRC_ENGINE_ETC_CONSTANTS_HPP

#include <cstdint>
#include <string>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

const uint32_t WIDTH = 2880;
const uint32_t HEIGHT = 1920;

const std::string MODEL_PATH = "res/models/viking_room.obj";
const std::string TEXTURE_PATH = "res/textures/perlin.png";


const std::string ENGINE_NAME = "";

const uint32_t ENGINE_MAJOR_VERSION = 0;
const uint32_t ENGINE_MINOR_VERSION = 1;
const uint32_t ENGINE_PATCH_VERSION = 0;


const std::vector<const char *> validation_layers 
= { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

#endif // SRC_ENGINE_ETC_CONSTANTS_HPP