#version 450 core

in FRAGMENT_ATTRIBUTES {
    vec4 position;
    vec4 color;
} fragAttr;

out vec4 outColor;

void main() {
    outColor = fragAttr.color;
}
