#version 450

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
//layout(location = 2) in float height;
layout(location = 4) in float inInstanceSize;

layout(binding = 1) uniform sampler2D noiseSampler;
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float scale = 1.0/32.0;
    //float h = 1 - height * scale;
    float c = inInstanceSize / 16.0;
    c *= 0.7;
    outColor = vec4(c, c, c, 1.0);
   // outColor = vec4(.5, .5, .5, 1.0);
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    //outColor = texture(texSampler, fragTexCoord);
}