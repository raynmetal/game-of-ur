/*
INCLUDE:
    - common/versionHeader.glsl
    - common/fragmentAttributes.vs
    - common/vertexAttributes.vs
*/

void main() {
    fragAttr.textureCoordinates = attrTextureCoordinates;
    gl_Position = attrPosition;
}
