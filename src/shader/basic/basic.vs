/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.vs
    - common/vertexAttributes.vs
*/

void main() {
    fragAttr.position = attrPosition;
    fragAttr.color = attrColor;
    fragAttr.UV1 = attrUV1;

    gl_Position = fragAttr.position;
}
