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
    if(attrLightEmission.mType != 0) { // Lights which are not directional require perspective transformations
        gl_Position = projectionMatrix * viewMatrix * attrModelMatrix * attrPosition;
    } else { // pass vertices as is, light covers the whole screen
        gl_Position = mat4(1.f) * attrPosition;
    }

    // Light placement calculations
    fragAttrLightPlacement.mPosition = viewMatrix * attrLightPlacement.mPosition;
    fragAttrLightPlacement.mDirection = viewMatrix * attrLightPlacement.mDirection;

    // Light emission, passed on as is
    fragAttrLightEmission = attrLightEmission;
}
