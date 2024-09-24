#ifndef ZOMATERIAL_H
#define ZOMATERIAL_H

#include <vector>
#include <string>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "resource_manager.hpp"
#include "texture_manager.hpp"

class Material : IResource {
public:
    virtual ~Material();
    Material();
    Material(const Material& other);
    Material(Material&& other);
    Material& operator=(const Material& other);
    Material& operator=(Material&& other);
    

    // TODO: not happy with all of the redundant code here, but I don't 
    // know how I'd go about making it more compact
    void updateFloatProperty(const std::string& name, float value);
    float getFloatProperty(const std::string& name);
    void updateIntProperty(const std::string& name, int value);
    int getIntProperty(const std::string& name);
    void updateVec2Property(const std::string& name, const glm::vec2& value);
    glm::vec2 getVec2Property(const std::string& name);
    void updateVec4Property(const std::string& name, const glm::vec4& value);
    glm::vec4 getVec4Property(const std::string& name);
    void updateTextureProperty(const std::string& name, const TextureHandle& value);
    TextureHandle getTextureProperty(const std::string& name);

    static void RegisterFloatProperty(const std::string& name, float defaultValue);
    static void RegisterIntProperty(const std::string& name, int defaultValue);
    static void RegisterVec4Property(const std::string& name, const glm::vec4& defaultValue);
    static void RegisterVec2Property(const std::string& name, const glm::vec2& defaultValue);
    static void RegisterTextureHandleProperty(const std::string& name, const TextureHandle& defaultValue);

    static void Init();

    static void Clear();

private:

    static Material* defaultMaterial;

    std::map<std::string, float> mFloatProperties {};
    std::map<std::string, int> mIntProperties {};
    std::map<std::string, glm::vec4> mVec4Properties {};
    std::map<std::string, glm::vec2> mVec2Properties {};
    std::map<std::string, TextureHandle> mTextureProperties {};

    virtual void destroyResource() override;
    virtual void releaseResource() override;

friend class ResourceManager<Material>;
};

#endif
