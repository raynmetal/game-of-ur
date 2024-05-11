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
    ShaderProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths);
    /*
    A constructor that reads and compiles a shader program
    from vertex, geometry, and fragment shader files
    */
    ShaderProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths, const std::vector<std::string>& geometryPaths);
    /*
    A constructor that reads and compiles a shader program
    from its JSON definition at the specified path
    */
    ShaderProgram(const std::string& programJSONPath);

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

    /* build shader using a set of vertex and fragment sources */
    void buildProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths);
    /* build shader using a set of vertex, fragment, and geometry sources */
    void buildProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths, const std::vector<std::string>& geometryPaths);

    /* Activate this shader */
    void use() const;
    /* Retrieve the build status of this shader*/
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
    GLuint mID {};
    bool mBuildState { false };
};

#endif
