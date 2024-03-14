#version 450

out FRAGMENT_ATTRIBUTES {
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

layout (std140) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

in mat4 attrModelMatrix;
in mat4 attrNormalMatrix;
in vec4 attrPosition;
in vec4 attrNormal;
in vec4 attrTangent;
in vec4 attrColor;
in vec2 attrTextureCoordinates;

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
