struct Light {

    vec4 mPosition;
    vec4 mDirection;

    vec4 mDiffuseColor;
    vec4 mSpecularColor;
    vec4 mAmbientColor;

    int mType; // 0-directional;  1-point;  2-spot;
    float mRadius;
    float mDecayLinear;
    float mDecayQuadratic;
    float mCosCutoffInner;
    float mCosCutoffOuter;
};