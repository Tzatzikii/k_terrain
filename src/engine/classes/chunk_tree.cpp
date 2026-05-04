#include "chunk_tree.hpp"

namespace ec {

ChunkTree::ChunkTree( int64_t _cx, int64_t _cy, int64_t _size, int64_t _level ) 
    : cx(_cx), cy(_cy), size(_size), level(_level)
{

    // Temporary magic number just to try out. 0 will be replaced with the player's position
    int64_t posx = 0;
    int64_t posy = 0;
    int64_t dx = std::abs(posx - cx);
    int64_t dy = std::abs(posy - cy);
    int64_t dist = std::sqrt( dx*dx + dy*dy );
    int64_t subdivide_dist = 3000/(std::pow(2, level));
    if(_level < ChunkTree::recursion_limit || (std::abs(cx - posx) > subdivide_dist ) ) {
        return;
    }
    int index = 0;

    for( int i = -1; i < 1; i+=2 ) {
        for( int j = -1; j < 1; j+=2 ) {
            
            int64_t next_cx = cx+i*size/4;
            int64_t next_cy = cy+i*size/4;
            int64_t next_size = size/4;
            std::unique_ptr<ChunkTree> next = std::make_unique<ChunkTree>( next_cx, next_cy, next_size, level+1 );
            std::swap( subdivide[index++], next );

        }
    }
}    

}