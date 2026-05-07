#version 450

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inChunkIndex;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint outChunkIndex;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    outChunkIndex = inChunkIndex;
}