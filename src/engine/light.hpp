#ifndef ZOLIGHT_H
#define ZOLIGHT_H

#include <utility>
#include <queue>
#include <map>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nlohmann/json.hpp>

#include "simple_ecs.hpp"
#include "shapegen.hpp"
#include "instance.hpp"
#include "scene_system.hpp"

struct LightEmissionData;

using LightPackedData = std::pair<std::pair<glm::vec4, glm::vec4>, LightEmissionData>;

struct LightEmissionData {
    static LightEmissionData MakeDirectionalLight(const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient);
    static LightEmissionData MakePointLight(const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);
    static LightEmissionData MakeSpotLight(
        float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient,
        float linearConst, float quadraticConst
    );

    enum LightType:int {
        directional=0,
        point=1,
        spot=2
    };

    LightType mType;
    glm::vec4 mDiffuseColor;
    glm::vec4 mSpecularColor;
    glm::vec4 mAmbientColor;

    //Attenuation attributes
    GLfloat mDecayLinear;
    GLfloat mDecayQuadratic;

    //Spotlight attributes
    GLfloat mCosCutoffInner;
    GLfloat mCosCutoffOuter;

    GLfloat mRadius {0.f};

    static float CalculateRadius(const glm::vec4& diffuseColor, float decayLinear, float decayQuadratic, float intensityCutoff);
    inline static std::string getComponentTypeName() { return "LightEmissionData"; }
};

NLOHMANN_JSON_SERIALIZE_ENUM( LightEmissionData::LightType, {
    {LightEmissionData::LightType::directional, "directional"},
    {LightEmissionData::LightType::point, "point"},
    {LightEmissionData::LightType::spot, "spot"},
})

inline void from_json(const nlohmann::json& json, LightEmissionData& lightEmissionData) {
    assert(json.at("type") == LightEmissionData::getComponentTypeName() && "Type mismatch. Light component json must have type LightEmissionData");
    json.at("lightType").get_to(lightEmissionData.mType);
    switch(lightEmissionData.mType) {
        case LightEmissionData::LightType::directional:
            lightEmissionData = LightEmissionData::MakeDirectionalLight(
                glm::vec3{
                    json.at("diffuse")[0].get<float>(),
                    json.at("diffuse")[1].get<float>(),
                    json.at("diffuse")[2].get<float>(),
                },
                glm::vec3{
                    json.at("specular")[0].get<float>(),
                    json.at("specular")[1].get<float>(),
                    json.at("specular")[2].get<float>(),
                },
                glm::vec3{
                    json.at("ambient")[0].get<float>(),
                    json.at("ambient")[1].get<float>(),
                    json.at("ambient")[2].get<float>(),
                }
            );
        break;
        case LightEmissionData::LightType::point:
            lightEmissionData = LightEmissionData::MakePointLight(
                glm::vec3{
                    json.at("diffuse")[0].get<float>(),
                    json.at("diffuse")[1].get<float>(),
                    json.at("diffuse")[2].get<float>(),
                },
                glm::vec3{
                    json.at("specular")[0].get<float>(),
                    json.at("specular")[1].get<float>(),
                    json.at("specular")[2].get<float>(),
                },
                glm::vec3{
                    json.at("ambient")[0].get<float>(),
                    json.at("ambient")[1].get<float>(),
                    json.at("ambient")[2].get<float>(),
                },
                json.at("linearConst").get<float>(),
                json.at("quadraticConst").get<float>()
            );
        break;
        case LightEmissionData::LightType::spot:
            lightEmissionData = LightEmissionData::MakeSpotLight(
                json.at("innerAngle").get<float>(),
                json.at("outerAngle").get<float>(),
                glm::vec3{
                    json.at("diffuse")[0].get<float>(),
                    json.at("diffuse")[1].get<float>(),
                    json.at("diffuse")[2].get<float>(),
                },
                glm::vec3{
                    json.at("specular")[0].get<float>(),
                    json.at("specular")[1].get<float>(),
                    json.at("specular")[2].get<float>(),
                },
                glm::vec3{
                    json.at("ambient")[0].get<float>(),
                    json.at("ambient")[1].get<float>(),
                    json.at("ambient")[2].get<float>(),
                },
                json.at("linearConst").get<float>(),
                json.at("quadraticConst").get<float>()
            );
        break;
    }
}
inline void to_json(nlohmann::json& json, const LightEmissionData& lightEmissionData) {
    switch(lightEmissionData.mType) {
        case LightEmissionData::LightType::directional:
            json = {
                {"type", LightEmissionData::getComponentTypeName()},
                {"lightType", lightEmissionData.mType},
                {"diffuse", {
                    lightEmissionData.mDiffuseColor.r,
                    lightEmissionData.mDiffuseColor.g,
                    lightEmissionData.mDiffuseColor.b,
                }},
                {"specular", {
                    lightEmissionData.mSpecularColor.r,
                    lightEmissionData.mSpecularColor.g,
                    lightEmissionData.mSpecularColor.b,
                }},
                {"ambient", {
                    lightEmissionData.mAmbientColor.r,
                    lightEmissionData.mAmbientColor.g,
                    lightEmissionData.mAmbientColor.b,
                }},
            };
        break;
        case LightEmissionData::LightType::point:
            json = {
                {"type", LightEmissionData::getComponentTypeName()},
                {"lightType", lightEmissionData.mType},
                {"diffuse", {
                    lightEmissionData.mDiffuseColor.r,
                    lightEmissionData.mDiffuseColor.g,
                    lightEmissionData.mDiffuseColor.b,
                }},
                {"specular", {
                    lightEmissionData.mSpecularColor.r,
                    lightEmissionData.mSpecularColor.g,
                    lightEmissionData.mSpecularColor.b,
                }},
                {"ambient", {
                    lightEmissionData.mAmbientColor.r,
                    lightEmissionData.mAmbientColor.g,
                    lightEmissionData.mAmbientColor.b,
                }},
                {"linearConst", lightEmissionData.mDecayLinear},
                {"quadraticConst", lightEmissionData.mDecayQuadratic},
            };
        break;
        case LightEmissionData::LightType::spot:
            json = {
                {"type", LightEmissionData::getComponentTypeName()},
                {"lightType", lightEmissionData.mType},
                {"diffuse", {
                    lightEmissionData.mDiffuseColor.r,
                    lightEmissionData.mDiffuseColor.g,
                    lightEmissionData.mDiffuseColor.b,
                }},
                {"specular", {
                    lightEmissionData.mSpecularColor.r,
                    lightEmissionData.mSpecularColor.g,
                    lightEmissionData.mSpecularColor.b,
                }},
                {"ambient", {
                    lightEmissionData.mAmbientColor.r,
                    lightEmissionData.mAmbientColor.g,
                    lightEmissionData.mAmbientColor.b,
                }},
                {"linearConst", lightEmissionData.mDecayLinear},
                {"quadraticConst", lightEmissionData.mDecayQuadratic},
                {"innerAngle", lightEmissionData.mCosCutoffInner},
                {"outerAngle", lightEmissionData.mCosCutoffOuter},
            };
        break;
    }
}

static InstanceLayout LightInstanceLayout {{
    {"attrLightPlacement.mPosition", RUNTIME, 4, GL_FLOAT},
    {"attrLightPlacement.mDirection", RUNTIME, 4, GL_FLOAT},

    {"attrLightEmission.mType", RUNTIME, 1, GL_INT},
    {"attrLightEmission.mDiffuseColor", RUNTIME, 4, GL_FLOAT},
    {"attrLightEmission.mSpecularColor", RUNTIME, 4, GL_FLOAT},
    {"attrLightEmission.mAmbientColor", RUNTIME, 4, GL_FLOAT},
    {"attrLightEmission.mDecayLinear", RUNTIME, 1, GL_FLOAT},
    {"attrLightEmission.mDecayQuadratic", RUNTIME, 1, GL_FLOAT},
    {"attrLightEmission.mCosCutoffInner", RUNTIME, 1, GL_FLOAT},
    {"attrLightEmission.mCosCutoffOuter", RUNTIME, 1, GL_FLOAT},
    {"attrLightEmission.mRadius", RUNTIME, 1, GL_FLOAT}
}};


class LightInstanceAllocator : public BaseInstanceAllocator {
public:
    LightInstanceAllocator(const std::vector<LightEmissionData>& lightEmissionDataList, const std::vector<glm::mat4>& lightModelMatrices);

protected:
    virtual void upload() override;

private:
    std::vector<LightPackedData> mLightData;
};

template<>
inline LightEmissionData Interpolator<LightEmissionData>::operator() (
    const LightEmissionData& previousState,
    const LightEmissionData& nextState,
    float simulationProgress
) const {
    simulationProgress = mProgressLimits(simulationProgress);
    LightEmissionData interpolatedState { previousState };

    interpolatedState.mDiffuseColor += simulationProgress * (nextState.mDiffuseColor - previousState.mDiffuseColor);
    interpolatedState.mSpecularColor += simulationProgress * (nextState.mSpecularColor - previousState.mSpecularColor);
    interpolatedState.mAmbientColor += simulationProgress * (nextState.mAmbientColor - previousState.mAmbientColor);
    interpolatedState.mDecayLinear += simulationProgress * (nextState.mDecayLinear - previousState.mDecayLinear);
    interpolatedState.mDecayQuadratic += simulationProgress * (nextState.mDecayQuadratic - previousState.mDecayQuadratic);
    interpolatedState.mCosCutoffInner += simulationProgress * (nextState.mCosCutoffInner - previousState.mCosCutoffInner);
    interpolatedState.mCosCutoffOuter += simulationProgress * (nextState.mCosCutoffOuter - previousState.mCosCutoffOuter);

    return interpolatedState;
}

#endif
