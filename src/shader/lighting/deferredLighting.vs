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
        gl_Position = attrModelMatrix * attrPosition;
    }

    // Light placement calculations
    fragAttrLightPlacement.mPosition = viewMatrix * attrLightPlacement.mPosition;
    fragAttrLightPlacement.mDirection = viewMatrix * attrLightPlacement.mDirection;

    // Light emission, passed on as is
    fragAttrLightEmission = attrLightEmission;
}
