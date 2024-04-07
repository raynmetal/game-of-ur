/*
Include:
    - common/versionHeader.glsl
    - common/vertexAttributes.vs
    - common/projectionViewMatrices.vs
    - common/modelNormalMatrices.vs
    - common/fragmentAttributes.vs
*/

void main() {
    // precomputed view and normal matrices
    mat4 viewModelMatrix = viewMatrix * attrModelMatrix;
    mat4 viewNormalMatrix = viewMatrix * attrNormalMatrix;

    // fragment attributes, in terms of camera space
    fragAttr.position = viewModelMatrix * attrPosition;
    fragAttr.normal = viewNormalMatrix * attrNormal;
    fragAttr.tangent = viewNormalMatrix * attrTangent;

    // color related fragment attributes
    fragAttr.textureCoordinates = attrTextureCoordinates;
    fragAttr.color = attrColor;

    // pre-NDC position, after projection and before viewport
    // transform is applied
    gl_Position = projectionMatrix * fragAttr.position;
}
