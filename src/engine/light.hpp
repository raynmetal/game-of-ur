#ifndef ZOLIGHT_H
#define ZOLIGHT_H

#include <queue>
#include <map>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh_manager.hpp"
#include "shapegen.hpp"
#include "resource_manager.hpp"
#include "instance.hpp"

struct LightData {
    static LightData MakeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse,  const glm::vec3& specular, const glm::vec3& ambient);
    static LightData MakePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);
    static LightData MakeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);

    enum LightType:int {
        directional=0,
        point=1,
        spot=2
    };

    //Basic light attributes
    LightType mType;
    glm::vec4 mPosition;
    glm::vec4 mDirection;
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
    {"lightPosition", LOCATION_LIGHTPOSITION, 4, GL_FLOAT},
    {"lightDirection", LOCATION_LIGHTDIRECTION, 4, GL_FLOAT},
    {"lightDiffuseColor", LOCATION_DIFFUSECOLOR, 4, GL_FLOAT},
    {"lightSpecularColor", LOCATION_SPECULARCOLOR, 4, GL_FLOAT},
    {"lightAmbientColor", LOCATION_AMBIENTCOLOR, 4, GL_FLOAT},
    {"lightType", LOCATION_LIGHTTYPE, 1, GL_INT},
    {"lightDecayLinear", LOCATION_LIGHTDECAYLINEAR, 1, GL_FLOAT},
    {"lightDecayQuadratic", LOCATION_LIGHTDECAYQUADRATIC, 1, GL_FLOAT},
    {"lightCosCutoffInner", LOCATION_LIGHTCOSCUTOFFINNER, 1, GL_FLOAT},
    {"lightCosCutoffOuter", LOCATION_LIGHTCOSCUTOFFOUTER, 1, GL_FLOAT}
}};

class LightInstanceAllocator : public BaseInstanceAllocator {
public:
    LightInstanceAllocator(const std::vector<LightData>& lightDataList);

protected:
    virtual void upload() override;

private:
    std::vector<LightData> mLightDataList;
};

#endif
