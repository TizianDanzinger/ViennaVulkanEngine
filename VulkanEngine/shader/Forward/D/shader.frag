#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../common_defines.glsl"
#include "../light.glsl"


layout(location = 0) in vec3 fragPosW;
layout(location = 1) in vec3 fragNormalW;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObjectPerFrame {
    mat4 camModel;
    mat4 camView;
    mat4 camProj;
    mat4 shadowView;
    mat4 shadowProj;
    light_t light1;
} UBOPerFrame;

layout(set = 2, binding = 0) uniform sampler2D shadowMap;

layout(set = 3, binding = 0) uniform sampler2D texSampler;

void main() {

    //parameters
    vec3 camPosW = UBOPerFrame.camModel[3].xyz;

    vec3 lightPosW = UBOPerFrame.light1.transform[3].xyz;
    vec3 lightDirW = normalize( UBOPerFrame.light1.transform[2].xyz );
    vec4 lightParam = UBOPerFrame.light1.param;
    float shadowFactor = shadowFunc( fragPosW, UBOPerFrame.shadowView, UBOPerFrame.shadowProj, shadowMap );

    vec3 ambcol  = UBOPerFrame.light1.col_ambient.xyz;
    vec3 diffcol = UBOPerFrame.light1.col_diffuse.xyz;
    vec3 speccol = UBOPerFrame.light1.col_specular.xyz;

    vec3 fragColor = texture(texSampler, fragTexCoord).xyz;

    vec3 result = vec3(0,0,0);

    result += UBOPerFrame.light1.itype[0] == 0?
                  dirlight( camPosW,
                            lightDirW, lightParam, shadowFactor,
                            ambcol, diffcol, speccol,
                            fragPosW, fragNormalW, fragColor) : vec3(0,0,0);

    result += UBOPerFrame.light1.itype[0] == 1?
                  pointlight( camPosW,
                              lightPosW, lightParam, shadowFactor,
                              ambcol, diffcol, speccol,
                              fragPosW, fragNormalW, fragColor) : vec3(0,0,0);

    result += UBOPerFrame.light1.itype[0] == 2?
                  spotlight( camPosW,
                             lightPosW, lightDirW, lightParam, shadowFactor,
                             ambcol, diffcol, speccol,
                             fragPosW, fragNormalW, fragColor) : vec3(0,0,0);

    outColor = vec4( result, 1.0 );
}
