/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
*/

uniform sampler2D uRenderTexture;

out vec4 outColor;

void main() {
    outColor = fragAttr.color * texture(uRenderTexture, fragAttr.textureCoordinates);
}
