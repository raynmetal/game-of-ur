/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
*/

uniform sampler2D uRenderTexture;
uniform float uExposure;

out vec4 outColor;

void main() {
    vec4 inColor = texture(uRenderTexture, fragAttr.textureCoordinates);
    outColor = 1.f - exp(uExposure * (-inColor));
}
