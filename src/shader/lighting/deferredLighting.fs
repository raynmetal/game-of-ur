/*
Include: 
    -common/versionHeader.glsl
    -common/lightStruct.glsl
    -common/geometrySampler.fs
    -common/fragmentAttributes.fs
*/

in LightPlacement fragAttrLightPlacement;
flat in LightEmission fragAttrLightEmission;

layout (location=0) out vec4 outColor;
layout (location=1) out vec4 brightColor;

void main() {
    vec2 textureCoordinates = gl_FragCoord.xy/vec2(uScreenWidth, uScreenHeight);

    // Sample gBuffer textures
    vec4 gPosition = texture(uGeometryPositionMap, textureCoordinates);
    vec4 gNormal = texture(uGeometryNormalMap, textureCoordinates);
    vec4 gAlbedoSpec = texture(uGeometryAlbedoSpecMap, textureCoordinates);
    vec4 gAlbedo = vec4(gAlbedoSpec.xyz, 1.f);
    vec4 gSpecular = vec4(vec3(gAlbedoSpec.w), 1.f);

    outColor = gPosition;
}
