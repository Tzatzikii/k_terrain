#include "engine_core.hpp"
#include "../classes/chunk_tree.hpp"
#include <memory>


namespace ec {

void BaseApp::load_model( std::string path ) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    if( !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str() ) ) {

        if( !warn.empty() ) {
            std::cout << "tinyobj warning: " << warn << std::endl;
        }
        throw std::runtime_error( err );
    }

    std::unordered_map<vertex, uint32_t> unique_vertices = {};
    
    for( const auto & shape : shapes ) {
        for( const auto & index : shape.mesh.indices ) {
            vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            vertex.tex_coord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
        
            if( unique_vertices.count( vertex ) == 0 ) {
                unique_vertices[vertex] = static_cast<uint32_t>( vertices.size() );
                //vertices.push_back( vertex );
            }
            
           // indices.push_back( unique_vertices[vertex] );
        }
    }


    this->chunk_tree = std::make_unique<ec::ChunkTree>( 0, 0, 65536 );
    chunk_tree->get_geometry( vertices, indices, 0 );
    
    std::cout << vertices.size() << std::endl;
}    

void BaseApp::load_models() {

    for( std::string path : model_paths ) {
        load_model( path );
    }

}

} // namespace ec