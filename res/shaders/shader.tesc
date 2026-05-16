#version 450

layout(vertices = 4) out; // triangle patches

layout(location = 1) in vec2 in_tex_coords[];
layout(location = 2) in uint in_chunk_index[];
layout(location = 1) out vec2 out_tex_coords[];
layout(location = 2) patch out uint out_chunk_index;

void main() {
    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    out_tex_coords[gl_InvocationID] = in_tex_coords[gl_InvocationID];
    float tessLevel = 16.0;

    // One invocation sets tessellation levels
    if (gl_InvocationID == 0) {
        out_chunk_index = in_chunk_index[0];
        // Outer edges
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16;
        gl_TessLevelOuter[3] = 8;

        // Inner (for triangles, only [0] is used)
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}

