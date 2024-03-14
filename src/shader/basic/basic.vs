#version 450 core

in vec4 attrPosition;
in vec4 attrColor;
in vec2 attrTextureCoordinates;

out FRAGMENT_ATTRIBUTES {
    vec4 position;
    vec4 color;
    vec2 textureCoordinates;
} fragAttr;

void main() {
    fragAttr.position = attrPosition;
    fragAttr.color = attrColor;
    fragAttr.textureCoordinates = attrTextureCoordinates;

    gl_Position = fragAttr.position;
}
