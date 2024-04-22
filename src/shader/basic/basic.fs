/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
    - common/genericSampler.fs
*/

out vec4 outColor;

void main() {
    outColor = fragAttr.color * texture(uGenericTexture, fragAttr.textureCoordinates);
}
