#ifndef SRC_ENGINE_CLASSES_DRAWABLE_HPP
#define SRC_ENGINE_CLASSES_DRAWABLE_HPP

#include <vector>
#include <stdint.h>
#include "../etc/header_libs.hpp"
#include "../math/vertex.hpp"


namespace ec {

class Drawable {
    std::vector<ec::vertex> vertices;
    std::vector<uint32_t> indices;
};

}

#endif