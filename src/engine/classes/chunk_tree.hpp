#ifndef RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP
#define RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP

#include <memory>
#include <stdint.h>
#include "../etc/header_libs.hpp"

namespace ec {

class ChunkTree {
private:
    std::unique_ptr<ChunkTree> subdivide[4] = {nullptr};
    int64_t max = INT64_MAX;
    int64_t min = INT64_MIN;
    static const int64_t recursion_limit = 20;
    int64_t level = 0;
    int64_t cx, cy;
    int64_t size;
    


public:
    ChunkTree( int64_t _cx, int64_t _cy, int64_t _size = INT64_MAX/2, int64_t _level = 0);
    glm::vec2 get_center() { return glm::vec2( static_cast<float>(cx), static_cast<float>(cy) ); }

};

}


#endif // RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP