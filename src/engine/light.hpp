#ifndef ZOLIGHT_H
#define ZOLIGHT_H

#include <utility>
#include <queue>
#include <map>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "placement.hpp"
#include "mesh_manager.hpp"
#include "shapegen.hpp"
#include "resource_manager.hpp"
#include "instance.hpp"

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
};

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
    LightInstanceAllocator(const std::vector<LightEmissionData>& lightEmissionDataList, const std::vector<Placement>& lightPlacementList);

protected:
    virtual void upload() override;

private:
    std::vector<LightPackedData> mLightData;
};


#endif
