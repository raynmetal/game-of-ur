/*
Include: 
    -common/versionHeader.glsl
    -common/lightStruct.glsl
    -common/fragmentAttributes.fs
*/

in LightPlacement fragAttrLightPlacement;
flat in LightEmission fragAttrLightEmission;

layout (location=0) out vec4 outColor;
layout (location=1) out vec4 brightColor;

void main() {
    // outColor = fragAttrLightEmission.mDiffuseColor;
    outColor = vec4(1.f);
}
