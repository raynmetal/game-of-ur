#ifndef ZOSHADERPROGRAM_H
#define ZOSHADERPROGRAM_H

#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "resource_database.hpp"

class ShaderProgram: public Resource<ShaderProgram> {
public:
    inline static std::string getResourceTypeName() { return "ShaderProgram"; }

    ShaderProgram(GLuint program);

    /*
    Shader destructor; deletes shader from memory
    */
    ~ShaderProgram();

    /* Shader copy constructor */
    ShaderProgram(const ShaderProgram& other) = delete;
    /* Shader copy assignment */
    ShaderProgram& operator=(const ShaderProgram& other) = delete;

    /* Shader move constructor */
    ShaderProgram(ShaderProgram&& other) noexcept;
    /* Shader move assignment */
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    /* Activate this shader */
    void use() const;

    //utility attrib array functions
    GLint getLocationAttribArray(const std::string& name) const;
    void enableAttribArray(const std::string& name) const;
    void enableAttribArray(GLint locationAttrib) const;
    void disableAttribArray(const std::string& name) const;
    void disableAttribArray(GLint locationAttrib) const;

    //utility uniform functions
    GLint getLocationUniform(const std::string& name) const;
    void setUBool(const std::string& name, bool value) const;
    void setUInt(const std::string& name, int value) const;
    void setUFloat(const std::string& name, float value) const;
    void setUVec3(const std::string& name, const glm::vec3& value) const;
    void setUVec4(const std::string& name, const glm::vec4& value) const;
    void setUMat4(const std::string& name, const glm::mat4& value) const;

    //utility uniform block functions
    GLint getLocationUniformBlock(const std::string& name) const;
    void setUniformBlock(const std::string& name, GLuint bindingPoint) const;

    GLuint getProgramID() const;

private:
    void destroyResource();
    void releaseResource();

    // program ID
    GLuint mID;
};

class ShaderProgramFromFile: public ResourceFactoryMethod<ShaderProgram, ShaderProgramFromFile> {
public:
    ShaderProgramFromFile():
    ResourceFactoryMethod<ShaderProgram, ShaderProgramFromFile> {0}
    {}
    inline static std::string getResourceConstructorName() { return "fromFile"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

#endif
