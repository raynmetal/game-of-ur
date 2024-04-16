#ifndef ZOLIGHT_H
#define ZOLIGHT_H

#include <queue>
#include <map>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_program.hpp"
#include "mesh.hpp"
#include "shapegen.hpp"

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

    float calculateRadius(float intensityCutoff) const;
};

class LightCollection {
public:
    /* default constructor */
    LightCollection();
    /* move constructor */
    LightCollection(LightCollection&& other);
    /* copy constructor */
    LightCollection(const LightCollection& other);
    /* move assignment operator */
    LightCollection& operator=(LightCollection&& other);
    /* copy assignment operator */
    LightCollection& operator=(const LightCollection& other);

    void draw(const ShaderProgram& shaderProgram);

    /* associates a shader program with this collection's light volume mesh */
    void associateShader(GLuint programID);
    /* disassociates a shader program from this collection's light volume mesh */
    void disassociateShader(GLuint programID);

    GLuint addLight(const Light& light);
    Light getLight(GLuint instanceID) const;
    void updateLight(GLuint instanceID, const Light& light);
    void removeLight(GLuint instanceID);

private:
    void updateBuffers();
    void allocateBuffers();
    void free();
    void stealResources(LightCollection& other);
    void copyResources(const LightCollection& other);

    GLuint mModelMatrixBuffer { 0 };
    GLuint mLightBuffer { 0 };
    Mesh mLightVolume {generateSphereMesh(10, 10)};

    GLuint mNextInstanceID {0};
    std::queue<GLuint> mDeletedInstanceIDs {};
    std::map<GLuint, glm::mat4> mInstanceModelMatrixMap {};
    std::map<GLuint, Light> mInstanceLightMap {};
    GLuint mInstanceCapacity { 128 };
    bool mDirty {true};
};

#endif
