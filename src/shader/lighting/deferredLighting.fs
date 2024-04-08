
/*
Include: 
    -common/versionHeader.glsl
    -common/lightStruct.glsl
    -common/fragmentAttributes.fs
*/

flat in Light fragAttrLight;

void main() {
    outColor = fragAttr.color * texture(uRenderTexture, fragAttr.textureCoordinates);
}

