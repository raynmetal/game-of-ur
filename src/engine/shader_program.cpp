#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_program.hpp"

GLint loadAndCompileShader(const std::vector<std::string>& shaderPaths, GLuint& shaderID, GLuint shaderType);
void freeProgram(GLuint programID);

ShaderProgram::ShaderProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths) :mBuildState{false} {
    GLuint vertexShader;
    GLuint fragmentShader;

    if(loadAndCompileShader(vertexPaths, vertexShader, GL_VERTEX_SHADER) != GL_TRUE) return;
    if(loadAndCompileShader(fragmentPaths, fragmentShader, GL_FRAGMENT_SHADER) != GL_TRUE){
        glDeleteShader(vertexShader);
        return;
    }

    //Create a shader program
    GLint success {};
    char infoLog[512];
    mID = glCreateProgram();
    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);
    glLinkProgram(mID);

    //Clean up (now unnecessary) shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Report failure, if it occurred
    glGetProgramiv(mID, GL_LINK_STATUS, &success);
    if(success != GL_TRUE) {
        glGetProgramInfoLog(mID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
        glDeleteProgram(mID);
        mID = 0;
        return;
    }

    // Store build success
    mBuildState = true;
    return;
}

ShaderProgram::ShaderProgram(const std::vector<std::string>& vertexPaths, const std::vector<std::string>& fragmentPaths, const std::vector<std::string>& geometryPaths) :mBuildState{false} {
    GLuint vertexShader {};
    GLuint fragmentShader {};
    GLuint geometryShader {};

    if(loadAndCompileShader(vertexPaths, vertexShader, GL_VERTEX_SHADER) != GL_TRUE) return;
    if(loadAndCompileShader(fragmentPaths, fragmentShader, GL_FRAGMENT_SHADER) != GL_TRUE) {
        glDeleteShader(vertexShader);
        return;
    }
    if(loadAndCompileShader(geometryPaths, geometryShader, GL_GEOMETRY_SHADER) != GL_TRUE) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    //Create a shader program
    GLint success {};
    char infoLog[512];   
    mID = glCreateProgram();
    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);
    glAttachShader(mID, geometryShader);
    glLinkProgram(mID);

    //Clean up (now unnecessary) shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);

    //Report failure, if it occurred
    glGetProgramiv(mID, GL_LINK_STATUS, &success);
    if(success != GL_TRUE) {
        glGetProgramInfoLog(mID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
        glDeleteProgram(mID);
        mID = 0;
        return;
    }

    // Store build success
    mBuildState = true;
    return;
}

GLint loadAndCompileShader(const std::vector<std::string>& shaderPaths, GLuint& shaderID, GLuint shaderType) {
    //Read shader text
    const char** shaderSource { new const char* [shaderPaths.size()] };
    std::vector<std::string> shaderStrings(shaderPaths.size());
    for(std::size_t i{0}; i < shaderPaths.size(); ++i) {
        //enable file operation exceptions
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        bool readSuccess {true};
        try {
            shaderFile.open(shaderPaths[i]);
            // Read file buffer's contents into string stream
            std::stringstream shaderStream {};
            shaderStream << shaderFile.rdbuf();
            //Close file handles
            shaderFile.close();
            //Convert stream into string
            shaderStrings[i] = shaderStream.str();
            shaderSource[i] = shaderStrings[i].c_str();
        } catch(std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << shaderPaths[i] << std::endl;
            readSuccess = false;
        }
        if(!readSuccess) { 
            delete[] shaderSource;
            return GL_FALSE;
        }
    }

    //Variables for storing compilation success
    GLint success;
    char infoLog[512];

    //create vertex shader
    shaderID = glCreateShader(shaderType);
    glShaderSource(
        shaderID,
        shaderPaths.size(),
        shaderSource,
        NULL
    );
    glCompileShader(shaderID);
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    //Source no longer required, so remove it
    delete[] shaderSource;
    if(success != GL_TRUE) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::string typeString {};
        switch(shaderType) {
            case GL_VERTEX_SHADER: typeString = "VERTEX"; break;
            case GL_GEOMETRY_SHADER: typeString = "GEOMETRY"; break;
            case GL_FRAGMENT_SHADER: typeString = "FRAGMENT"; break;
            default: typeString = "UNKNOWN"; break;
        }
        std::cout << "ERROR::SHADER::" << typeString << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shaderID);
        shaderID = 0;
    }

    return success;
}

ShaderProgram::~ShaderProgram() {
    freeProgram(mID);
}

ShaderProgram::ShaderProgram(const ShaderProgram& other) :
    mID{other.mID},
    mBuildState{other.mBuildState}
{}
ShaderProgram& ShaderProgram::operator=(const ShaderProgram& other){
    if(&other == this) return *this;

    //Free currently held resource
    freeProgram(mID);

    //Copy other's resource
    mID = other.mID;
    mBuildState = other.mBuildState;

    return *this;
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept :
    mID {other.mID},
    mBuildState{other.mBuildState}
{
    // Prevent other from destroying resource 
    // when its destructor is called
    other.mID = 0;
}
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    // Do nothing on self assignment
    if(&other == this) return *this;

    // Free currently held resource
    freeProgram(mID);

    //Copy other
    mID = other.mID;
    mBuildState = other.mBuildState;

    //Prevent other from destroying moved resource when its
    //destructor is called
    other.mID = 0;

    return *this;
}

GLuint ShaderProgram::getProgramID() const { return mID; }
void ShaderProgram::use() const { glUseProgram(mID); }
bool ShaderProgram::getBuildSuccess() const { return mBuildState; }

GLint ShaderProgram::getLocationAttribArray(const std::string& name) const {
    return glGetAttribLocation(mID, name.c_str());
}
GLint ShaderProgram::getLocationUniform(const std::string& name) const {
    return glGetUniformLocation(mID, name.c_str());
}
GLint ShaderProgram::getLocationUniformBlock(const std::string& name) const {
    return glGetUniformBlockIndex(mID, name.c_str());
}

void ShaderProgram::enableAttribArray(const std::string& name) const {
    GLint loc {getLocationAttribArray(name)};
    if(loc<0) return;
    glEnableVertexAttribArray(loc);
}
void ShaderProgram::enableAttribArray(GLint locationAttrib) const {
    if(locationAttrib<0) return;
    glEnableVertexAttribArray(locationAttrib);
}
void ShaderProgram::disableAttribArray(const std::string& name) const {
    GLint loc {getLocationAttribArray(name)};
    if(loc<0) return;
    glDisableVertexAttribArray(loc);
}
void ShaderProgram::disableAttribArray(GLint locationAttrib) const {
    if(locationAttrib<0) return;
    glEnableVertexAttribArray(locationAttrib);
}

void ShaderProgram::setUBool(const std::string& name, bool value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniform1i(
        loc,
        static_cast<GLint>(value)
    );
}
void ShaderProgram::setUInt(const std::string& name, int value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniform1i(
        loc,
        static_cast<GLint>(value)
    );
}
void ShaderProgram::setUFloat(const std::string& name, float value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniform1f(
        loc,
        static_cast<GLfloat>(value)
    );
}
void ShaderProgram::setUVec3(const std::string& name, const glm::vec3& value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniform3fv(
        loc,
        1,
        glm::value_ptr(value)
    );
}
void ShaderProgram::setUVec4(const std::string& name, const glm::vec4& value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniform4fv(
        loc,
        1,
        glm::value_ptr(value)
    );
}
void ShaderProgram::setUMat4(const std::string& name, const glm::mat4& value) const {
    GLint loc {getLocationUniform(name)};
    if(loc<0) return;
    glUniformMatrix4fv(
        loc,
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}
void ShaderProgram::setUniformBlock(const std::string& name, GLuint bindingPoint) const {
    GLint loc {getLocationUniformBlock(name)};
    if(loc<0) return;
    glUniformBlockBinding(
        mID,
        loc,
        bindingPoint
    );
}

void freeProgram(GLuint programID) {
    if(programID)
        glDeleteProgram(programID);
}