#version 450

struct Material {
    sampler2D textureAlbedo;
    sampler2D textureSpecular;
    sampler2D textureNormal;
};

out layout(location=0) vec4 geometryPosition;
out layout(location=1) vec4 geometryNormal;
out layout(location=2) vec4 geometryAlbedoSpec;

in FRAGMENT_ATTRIBUTES {
    // texture coordinates for this fragment
    vec2 textureCoordinates;
    // world position of the associated fragment
    vec4 position;
    // color of the associated fragment
    vec4 color;
    // normal vector for the associated fragment
    vec4 normal;
    // tangent vector for the associated fragment
    vec4 tangent;
} fragAttr;

uniform bool uUsingNormalMap;
uniform bool uUsingAlbedoMap;
uniform bool uUsingSpecularMap;
uniform Material uMaterial;

void main() {
    geometryPosition = fragAttr.position;
    // geometryPosition = vec4(1.f);

    geometryNormal = normalize(fragAttr.normal);
    if(uUsingNormalMap) {
        vec4 tangent = normalize(fragAttr.tangent - dot(fragAttr.tangent, geometryNormal) * geometryNormal);
        vec4 bitangent = vec4(cross(geometryNormal.xyz, tangent.xyz), 0.f);
        mat4 tbnMatrix = mat4(tangent, bitangent, geometryNormal, vec4(0.f, 0.f, 0.f, 1.f));

        //recompute gBuffer normal with the one we've read from the normal map
        geometryNormal = tbnMatrix * texture(uMaterial.textureNormal, fragAttr.textureCoordinates) * 2.f - 1.f;
    }

    geometryAlbedoSpec = fragAttr.color;
    if(uUsingAlbedoMap) {
        geometryAlbedoSpec = vec4(texture(uMaterial.textureAlbedo, fragAttr.textureCoordinates).rgb, 1.f);
    }
    if(uUsingSpecularMap) {
        geometryAlbedoSpec.a = texture(uMaterial.textureSpecular, fragAttr.textureCoordinates).r;
    }
}
