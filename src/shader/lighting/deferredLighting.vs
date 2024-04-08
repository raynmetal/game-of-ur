/*
Include:
    -common/versionHeader.glsl
    -common/lightStruct.glsl
    -common/projectionViewMatrices.vs
    -common/fragmentAttributes.vs
    -common/vertexAttributes.vs
*/

in Light attrLight;
flat out Light fragAttrLight;

void main() {
    fragAttr.position = attrPosition;
    fragAttr.textureCoordinates = attrTextureCoordinates;

    //incoming coordinates are already in NDC
    gl_Position = fragAttr.position;
}
