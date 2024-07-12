#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

#include "light.hpp"

float LightData::CalculateRadius(const glm::vec4& diffuseColor, float decayLinear, float decayQuadratic, float intensityCutoff) {
    float intensityMax = diffuseColor.r;
    if(diffuseColor.g > intensityMax) intensityMax = diffuseColor.g;
    if(diffuseColor.b > intensityMax) intensityMax = diffuseColor.b;
    float radiusCutoff {
        (
            -decayLinear
            + std::sqrt(
                decayLinear*decayLinear - 4.f * decayQuadratic * (1.f - intensityMax / intensityCutoff)
            )
        )
        / (2.f * decayQuadratic)
    };
    return radiusCutoff;
}

LightData LightData::MakeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient){
    return LightData{
        .mDirection { direction, 0.f },
        .mType { LightData::directional },
        .mDiffuseColor { diffuse, 1.f },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {0.f},
        .mDecayQuadratic {0.f},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

LightData LightData::MakePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst){
    return LightData{
        .mPosition { position, 1.f },
        .mType { LightData::point },
        .mDiffuseColor { diffuse, 1.f, },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {linearConst},
        .mDecayQuadratic {quadraticConst},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f},
        .mRadius {CalculateRadius({diffuse, 1.f}, linearConst, quadraticConst, .05f)}
    };
}

LightData LightData::MakeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst) {
    return LightData{
        .mPosition { position, 1.f },
        .mDirection { direction, 0.f },
        .mType { LightData::spot },
        .mDiffuseColor { diffuse, 1.f },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {linearConst},
        .mDecayQuadratic {quadraticConst},
        .mCosCutoffInner {glm::cos(glm::radians(innerAngle))},
        .mCosCutoffOuter {glm::cos(glm::radians(outerAngle))},
        .mRadius {CalculateRadius({diffuse, 1.f}, linearConst, quadraticConst, .05f)}
    };
}

LightInstanceAllocator::LightInstanceAllocator(const std::vector<LightData>& lightDataList)
:
    BaseInstanceAllocator { LightInstanceLayout },
    mLightDataList { lightDataList }
{}

void LightInstanceAllocator::upload() {
    if(isUploaded()) return;

    glGenBuffers(1, &mVertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER, mLightDataList.size() * sizeof(LightData), mLightDataList.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
