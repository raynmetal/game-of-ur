#ifndef ZOLIGHT_H
#define ZOLIGHT_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Light {

    static Light MakeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse,  const glm::vec3& specular, const glm::vec3& ambient);
    static Light MakePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);
    static Light MakeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);

    enum LightType:int {
        directional=0,
        point=1,
        spot=2
    };

    //Basic light attributes
    LightType mType;
    glm::vec3 mPosition;
    glm::vec3 mDirection;
    glm::vec3 mDiffuse;
    glm::vec3 mSpecular;
    glm::vec3 mAmbient;

    //Attenuation attributes
    GLfloat mConstant;
    GLfloat mLinear;
    GLfloat mQuadratic;

    //Spotlight attributes
    GLfloat mCosCutoffInner;
    GLfloat mCosCutoffOuter;

    float calculateRadius(float intensityCutoff);


};

Light makeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse,  const glm::vec3& specular, const glm::vec3& ambient);
Light makePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);
Light makeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst);

#endif
