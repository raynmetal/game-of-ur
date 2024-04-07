/*
Include:
    -common/versionHeader.glsl
*/

void main() {
    fragAttr.position = attrPosition;
    fragAttr.color = attrColor;
    fragAttr.textureCoordinates = attrTextureCoordinates;

    //incoming coordinates are already in NDC
    gl_Position = fragAttr.position;
}
