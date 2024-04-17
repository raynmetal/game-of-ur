/*
Include:
    -common/versionHeader.glsl
    -common/lightStruct.glsl
    -common/projectionViewMatrices.vs
    -common/modelNormalMatrices.vs
    -common/fragmentAttributes.vs
    -common/vertexAttributes.vs
*/

in LightPlacement attrLightPlacement;
in LightEmission attrLightEmission;

out LightPlacement fragAttrLightPlacement;
flat out LightEmission fragAttrLightEmission;


void main() {
    // Light volume calculations
    fragAttr.position = viewMatrix * attrModelMatrix * attrPosition;
    gl_Position = projectionMatrix * viewMatrix * attrModelMatrix * attrPosition;

    // Convert to NDC, then to gBuffer sampling coordinates
    fragAttr.textureCoordinates = (gl_Position.xy/gl_Position.w + 1.f)/2.f;

    // Light placement calculations
    fragAttrLightPlacement.mPosition = viewMatrix * attrLightPlacement.mPosition;
    fragAttrLightPlacement.mDirection = viewMatrix * attrLightPlacement.mDirection;

    // Light emission, passed on as is
    fragAttrLightEmission = attrLightEmission;
}
