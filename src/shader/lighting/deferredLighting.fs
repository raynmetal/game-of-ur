
/*
Include: 
    -common/versionHeader.glsl
*/

void main() {
    outColor = fragAttr.color * texture(uRenderTexture, fragAttr.textureCoordinates);
}

