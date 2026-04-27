#ifndef SRC_ENGINE_MATH_CAMERA_HPP
#define SRC_ENGINE_MATH_CAMERA_HPP

#include "../etc/header_libs.hpp"

namespace ec {

class Camera {
    glm::vec3 pos;
    glm::vec3 dir;

public:
    Camera( glm::vec3 initial_pos );
    void forwards( float units );
    void sideways( float units );

};

} // namespace ec

#endif // SRC_ENGINE_MATH_CAMERA_HPP