#version 450 core

in vec4 attrPosition;
in vec4 attrColor;

out FRAGMENT_ATTRIBUTES {
    vec4 position;
    vec4 color;
} fragAttr;

void main() {
    fragAttr.position = attrPosition;
    fragAttr.color = attrColor;

    gl_Position = fragAttr.position;
}
