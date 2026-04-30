#include "camera.hpp"

namespace ec {

void Camera::forwards( float units ) {
    glm::vec3 d = glm::vec3( dir );
    this->pos = glm::translate( glm::identity<glm::mat4>(), glm::normalize( d ) * units * speed ) * this->pos;
}

void Camera::sideways( float units ) {
    glm::vec3 n = this->get_normal();
    this->pos = glm::translate( glm::identity<glm::mat4>(), glm::normalize( n ) * units * speed ) * this->pos;
}

void Camera::rotate( float up, float side ) {
    glm::vec3 n = this->get_normal();
    this->dir = glm::rotate( glm::rotate( glm::identity<glm::mat4>(), up/100.0f, this->up ), side/100.0f, n  ) * this->dir;
}

} // namespace ec