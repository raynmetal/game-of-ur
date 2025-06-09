/*
Include:
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
    - common/material.fs
*/

out layout(location=0) vec4 geometryPosition;
out layout(location=1) vec4 geometryNormal;
out layout(location=2) vec4 geometryAlbedoSpec;

void main() {
    geometryPosition = fragAttr.position;
    // geometryPosition = vec4(1.f);

    geometryNormal = normalize(fragAttr.normal);
    if(uMaterial.mUsingNormalMap) {
        vec4 tangent = normalize(fragAttr.tangent - dot(fragAttr.tangent, geometryNormal) * geometryNormal);
        vec4 bitangent = vec4(cross(geometryNormal.xyz, tangent.xyz), 0.f);
        mat4 tbnMatrix = mat4(tangent, bitangent, geometryNormal, vec4(0.f, 0.f, 0.f, 1.f));

        //recompute gBuffer normal with the one we've read from the normal map
        geometryNormal = tbnMatrix * (texture(uMaterial.mTextureNormal, fragAttr.UV1) * 2.f - 1.f);
    }

    geometryAlbedoSpec = uMaterial.mColorMultiplier * fragAttr.color;
    if(uMaterial.mUsingAlbedoMap) {
        geometryAlbedoSpec = uMaterial.mColorMultiplier * vec4(texture(uMaterial.mTextureAlbedo, fragAttr.UV1).rgb, 0.1f);
    }
    if(uMaterial.mUsingSpecularMap) {
        geometryAlbedoSpec.a = texture(uMaterial.mTextureSpecular, fragAttr.UV1).r * uMaterial.mColorMultiplier.a;
    }
}
