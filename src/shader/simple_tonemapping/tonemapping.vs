/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.vs
    - common/vertexAttributes.vs
*/

void main() {
    fragAttr.position = attrPosition;
    fragAttr.textureCoordinates = attrTextureCoordinates;

    gl_Position = fragAttr.position;
}
