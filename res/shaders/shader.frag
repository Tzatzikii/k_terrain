#version 450

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 2) in float height;

layout(binding = 1) uniform sampler2D noiseSampler;
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float scale = 1.0/32.0;
    float h = 1 - height * scale;
    float c = height * 0.001;
    //outColor = vec4(c, c, c, 1.0);
    outColor = vec4(.5, .5, .5, 1.0);
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    //outColor = texture(texSampler, fragTexCoord);
}