#ifndef SRC_ENGINE_MATH_CAMERA_HPP
#define SRC_ENGINE_MATH_CAMERA_HPP

#include "../etc/header_libs.hpp"

namespace ec {

class Camera {
    glm::vec4 pos;
    glm::vec4 dir;
    glm::vec3 up = {0, 0, 1};
    float speed = 0.01f;

public:
    Camera( glm::vec3 _pos = {0, 0, 5.0f}, glm::vec3 _dir = {0, -1.0f, 0} )
        : pos(glm::vec4(_pos, 1)), dir(glm::vec4(_dir, 0)){}
    void forwards( float units );
    void sideways( float units );
    void vertical( float units );
    void rotate( float up, float side );
    glm::vec3 get_pos() { return glm::vec3{pos.x, pos.y, pos.z}; }
    glm::vec3 get_dir() { return dir; }
    glm::vec4 get_pos4() { return pos; }
    glm::vec4 get_dir4() { return dir; }
    glm::vec3 get_normal() { return glm::cross( this->get_dir(), this->up ); }

};

} // namespace ec

#endif // SRC_ENGINE_MATH_CAMERA_HPP