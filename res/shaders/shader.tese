#version 450
#extension GL_EXT_nonuniform_qualifier : enable
precision highp float;
precision highp sampler2D;

layout(quads, fractional_even_spacing, ccw) in;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) patch in uint inInstanceIndex;
layout(location = 3) patch in vec2 inInstanceCenter;
layout(location = 4) patch in float inInstanceSize;

layout(location = 1) out vec2 fragTexCoord;
//layout(location = 2) out float height;
layout(location = 2) flat out uint outInstanceIndex;
layout(location = 4) out float outInstanceSize;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2DArray noises;


void main() {
    outInstanceIndex = inInstanceIndex;
    outInstanceSize = inInstanceSize;
    float u = clamp(gl_TessCoord.x, 0.0, 1.0);
    float v = clamp(gl_TessCoord.y, 0.0, 1.0);

    fragTexCoord = vec2(u, v);

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    // retrieve control point texture coordinates
    vec2 t00 = inTexCoord[0];
    vec2 t01 = inTexCoord[1];
    vec2 t10 = inTexCoord[2];
    vec2 t11 = inTexCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    fragTexCoord = texCoord;
    float dist = sqrt( p.x*p.x + p.y*p.y );

    // lookup texel at patch coordinate for height and scale + shift as desired
    //height = texture(noises[inInstanceIndex], texCoord).r * 256.0;
    float height = texture(noises, vec3(texCoord, float(inInstanceIndex))).r * 256.0;
    //height = pow(height * 64.0, 2.0) - 256.0;
    //float height = -5.0;
    //height = floor(height);
    //height = 0;
    //height = dist/2.0;

    // compute patch surface normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(uVec.xyz, vVec.xyz), 0) );

    p.z += height; //+ log(float(inInstanceIndex));
    p.xy += inInstanceCenter;
    
    gl_Position = mvp.proj * mvp.view * p;


}