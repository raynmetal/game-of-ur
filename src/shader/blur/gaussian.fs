/*
INCLUDE
    - common/versionHeader.glsl
    - common/fragmentAttributes.fs
    - common/genericSampler.fs
*/


uniform bool uHorizontal;
uniform float weights[5] = {.227027f, .1945946f, .1216216f, .054054f, .016216f};

out vec4 outColor;

void main() {
    vec2 offset;
    if(uHorizontal) {
        offset = vec2(1.f, 0.f);
    } else {
        offset = vec2(0.f, 1.f);
    }
    offset /= textureSize(uGenericTexture, 0);

    outColor = weights[0] * texture(uGenericTexture, fragAttr.UV1);
    for(int i = 1; i < 5; ++i) {
        outColor += weights[i] * texture(uGenericTexture,
            i * offset + fragAttr.UV1
        );
        outColor += weights[i] * texture(uGenericTexture,
            -i * offset + fragAttr.UV1
        );
    }
}
