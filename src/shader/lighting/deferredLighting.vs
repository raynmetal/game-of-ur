/*
Include:
    -common/versionHeader.glsl
    -common/lightStruct.vs
    -common/projectionViewMatrices.vs
    -common/modelNormalMatrices.vs
    -common/fragmentAttributes.vs
    -common/vertexAttributes.vs
*/


void main() {
    // Light volume calculations
    gl_Position = projectionMatrix * viewMatrix * attrModelMatrix * attrPosition;
    if(attrLightEmission.mType == 0) { // directional lights, ignore matrices and pass attrPosition as is
        // TODO: hack upon hack. scale the sphere so that it covers the entire screen
        mat4 modelMatrix = mat4(1.f);
        modelMatrix[0][0] = sqrt(2.f);
        modelMatrix[1][1] = sqrt(2.f);
        gl_Position = modelMatrix * attrPosition;
    }

    // Light placement calculations
    fragAttrLightPlacement.mPosition = viewMatrix * attrLightPlacement.mPosition;
    fragAttrLightPlacement.mDirection = viewMatrix * attrLightPlacement.mDirection;

    // Light emission, passed on as is
    fragAttrLightEmission = attrLightEmission;
}
