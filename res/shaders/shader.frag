#version 450

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 2) in float height;

layout(binding = 1) uniform sampler2D noiseSampler;
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float scale = 1.0/16.0;
    float h = height * scale;
    outColor = texture(texSampler, fragTexCoord);
}