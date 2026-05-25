#include "engine_core.hpp"
#include "../classes/chunk_tree.hpp"
#include <memory>


namespace ec {

void BaseApp::load_model( std::string path ) {
    // tinyobj::attrib_t attrib;
    // std::vector<tinyobj::shape_t> shapes;
    // std::vector<tinyobj::material_t> materials;
    // std::string warn;
    // std::string err;

    // if( !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str() ) ) {

    //     if( !warn.empty() ) {
    //         std::cout << "tinyobj warning: " << warn << std::endl;
    //     }
    //     throw std::runtime_error( err );
    // }

    // std::unordered_map<vertex, uint32_t> unique_vertices = {};
    
    // for( const auto & shape : shapes ) {
    //     for( const auto & index : shape.mesh.indices ) {
    //         vertex vertex{};

    //         vertex.pos = {
    //             attrib.vertices[3 * index.vertex_index + 0],
    //             attrib.vertices[3 * index.vertex_index + 1],
    //             attrib.vertices[3 * index.vertex_index + 2]
    //         };
            
    //         vertex.tex_coord = {
    //             attrib.texcoords[2 * index.texcoord_index + 0],
    //             1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
    //         };
        
    //         if( unique_vertices.count( vertex ) == 0 ) {
    //             unique_vertices[vertex] = static_cast<uint32_t>( vertices.size() );
    //             //vertices.push_back( vertex );
    //         }
            
    //        // indices.push_back( unique_vertices[vertex] );
    //     }
    // }

    vertices.push_back( vertex{glm::vec3(-0.5, -0.5, 0), glm::vec2(0, 0)});
    vertices.push_back( vertex{glm::vec3(-0.5, 0.5, 0), glm::vec2(0, 1)});
    vertices.push_back( vertex{glm::vec3(0.5, -0.5, 0), glm::vec2(1, 0)});
    vertices.push_back( vertex{glm::vec3(0.5, 0.5, 0), glm::vec2(1, 1)});
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    // this->chunk_tree = std::make_unique<ec::QuadTree>( 0, 0, 1024 );
    // chunk_tree->update( this->camera.get_pos() );
    // chunk_tree->get_geometry( vertices, indices, 0 );
    
    //std::cout << vertices.size() << std::endl;
}    

void BaseApp::load_models() {

    for( std::string path : model_paths ) {
        load_model( path );
    }

}

void BaseApp::update_terrain() {
    
}

} // namespace ec