#version 450 core

uniform sampler2D uRenderTexture;

in FRAGMENT_ATTRIBUTES {
    vec4 position;
    vec4 color;
    vec2 textureCoordinates;
} fragAttr;

out vec4 outColor;

void main() {
    outColor = fragAttr.color * texture(uRenderTexture, fragAttr.textureCoordinates);
}
