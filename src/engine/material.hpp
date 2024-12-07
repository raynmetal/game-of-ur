#ifndef ZOMATERIAL_H
#define ZOMATERIAL_H

#include <vector>
#include <string>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "texture.hpp"
#include "resource_database.hpp"

class Material : public Resource<Material> {
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
    void updateTextureProperty(const std::string& name, std::shared_ptr<Texture> value);
    std::shared_ptr<Texture> getTextureProperty(const std::string& name);

    static void RegisterFloatProperty(const std::string& name, float defaultValue);
    static void RegisterIntProperty(const std::string& name, int defaultValue);
    static void RegisterVec4Property(const std::string& name, const glm::vec4& defaultValue);
    static void RegisterVec2Property(const std::string& name, const glm::vec2& defaultValue);
    static void RegisterTextureHandleProperty(const std::string& name, std::shared_ptr<Texture> defaultValue);

    inline static std::string getResourceTypeName() { return "Material"; }

    static void Init();

    static void Clear();

private:

    static Material* defaultMaterial;

    std::map<std::string, float> mFloatProperties {};
    std::map<std::string, int> mIntProperties {};
    std::map<std::string, glm::vec4> mVec4Properties {};
    std::map<std::string, glm::vec2> mVec2Properties {};
    std::map<std::string, std::shared_ptr<Texture>> mTextureProperties {};

    void destroyResource();
    void releaseResource();
};

class MaterialFromDescription: public ResourceFactoryMethod<Material, MaterialFromDescription> {
public:

    MaterialFromDescription():
    ResourceFactoryMethod<Material, MaterialFromDescription> {0}
    {}
    inline static std::string getResourceConstructorName() { return "fromDescription"; }

private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

#endif
