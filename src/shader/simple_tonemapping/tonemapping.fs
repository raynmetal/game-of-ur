/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
    - common/genericSampler.fs
*/

uniform float uExposure;
uniform bool uCombine;

out vec4 outColor;

void main() {
    vec4 inColor = texture(uGenericTexture, fragAttr.textureCoordinates);
    if(uCombine) inColor += texture(uGenericTexture1, fragAttr.textureCoordinates);
    outColor = 1.f - exp(uExposure * (-inColor));
}
