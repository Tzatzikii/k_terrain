#version 450

layout(triangles, equal_spacing, cw) in;

layout(location = 1) in vec2 inTexCoord[];

layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Barycentric interpolation
    vec3 bary = gl_TessCoord;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    gl_Position =
        bary.x * p0 +
        bary.y * p1 +
        bary.z * p2;

    fragTexCoord = 
        bary.x * inTexCoord[0] +
        bary.y * inTexCoord[1] +
        bary.z * inTexCoord[2];
    
}