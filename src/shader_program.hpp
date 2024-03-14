#ifndef ZOSHADERPROGRAM_H
#define ZOSHADERPROGRAM_H

#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

class ShaderProgram {
public:
    /*
    A constructor that reads and compiles a shader program 
    from vertex and fragment shader files
    */
    ShaderProgram(const char* vertexPath, const char* fragmentPath);
    /*
    A constructor that reads and compiles a shader program
    from vertex, geometry, and fragment shader files
    */
    ShaderProgram(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
    /*
    Shader destructor; deletes shader from memory
    */
    ~ShaderProgram();

    /* Shader copy constructor */
    ShaderProgram(const ShaderProgram& other);

    /* Shader copy assignment */
    ShaderProgram& operator=(const ShaderProgram& other);

    /* Shader move constructor */
    ShaderProgram(ShaderProgram&& other) noexcept;

    /* Shader move assignment */
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    //use/activate this shader
    void use() const;
    bool getBuildSuccess() const;

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
    // program ID
    GLuint mID;
    bool mBuildState;
};

#endif
