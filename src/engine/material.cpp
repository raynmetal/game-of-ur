#include <string>
#include <map>

#include <glm/glm.hpp>

#include "material.hpp"

Material* Material::defaultMaterial { nullptr };

Material::~Material() { destroyResource(); }
Material::Material() {}
Material::Material(const Material& other):
    mFloatProperties{other.mFloatProperties}, mIntProperties{other.mIntProperties},
    mVec4Properties{other.mVec4Properties}, mVec2Properties{other.mVec2Properties},
    mTextureProperties{other.mTextureProperties}
{}
Material::Material(Material&& other):
    mFloatProperties{other.mFloatProperties}, mIntProperties{other.mIntProperties},
    mVec4Properties{other.mVec4Properties}, mVec2Properties{other.mVec2Properties},
    mTextureProperties{other.mTextureProperties}
{ other.releaseResource(); }
Material& Material::operator=(const Material& other) {
    if(this == &other) return *this;
    mFloatProperties = other.mFloatProperties;
    mIntProperties = other.mIntProperties;
    mVec2Properties = other.mVec2Properties;
    mVec4Properties = other.mVec4Properties;
    mTextureProperties = other.mTextureProperties;
    return *this;
}
Material& Material::operator=(Material&& other) {
    if(this == &other) return *this;
    std::swap(mFloatProperties, other.mFloatProperties);
    std::swap(mIntProperties, other.mIntProperties);
    std::swap(mVec2Properties, other.mVec2Properties);
    std::swap(mVec4Properties, other.mVec4Properties);
    std::swap(mTextureProperties, other.mTextureProperties);
    other.releaseResource();
    return *this;
}

// TODO: not happy with all of the redundant code here, but I don't 
// know how I'd go about making it more compact
void Material::updateFloatProperty(const std::string& name, float value) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mFloatProperties.find(name) != Material::defaultMaterial->mFloatProperties.end());
    mFloatProperties[name] = value;
}
float Material::getFloatProperty(const std::string& name) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mFloatProperties.find(name) != Material::defaultMaterial->mFloatProperties.end());
    if(mFloatProperties.find(name) != mFloatProperties.end())
        return mFloatProperties[name];
    return Material::defaultMaterial->mFloatProperties[name];
}
void Material::updateIntProperty(const std::string& name, int value) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mIntProperties.find(name) != Material::defaultMaterial->mIntProperties.end());
    mIntProperties[name] = value;
}
int Material::getIntProperty(const std::string& name) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mIntProperties.find(name) != Material::defaultMaterial->mIntProperties.end());
    if(mIntProperties.find(name) != mIntProperties.end())
        return mIntProperties[name];
    return Material::defaultMaterial->mIntProperties[name];
}
void Material::updateVec2Property(const std::string& name, const glm::vec2& value) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mVec2Properties.find(name) != Material::defaultMaterial->mVec2Properties.end());
    mVec2Properties[name] = value;
}
glm::vec2 Material::getVec2Property(const std::string& name) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mVec2Properties.find(name) != Material::defaultMaterial->mVec2Properties.end());
    if(mVec2Properties.find(name) != mVec2Properties.end())
        return mVec2Properties[name];
    return Material::defaultMaterial->mVec2Properties[name];
}
void Material::updateVec4Property(const std::string& name, const glm::vec4& value) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mVec4Properties.find(name) != Material::defaultMaterial->mVec4Properties.end());
    mVec4Properties[name] = value;
}
glm::vec4 Material::getVec4Property(const std::string& name) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mVec4Properties.find(name) != Material::defaultMaterial->mVec4Properties.end());
    if(mVec4Properties.find(name) != mVec4Properties.end())
        return mVec4Properties[name];
    return Material::defaultMaterial->mVec4Properties[name];
}
void Material::updateTextureProperty(const std::string& name, const TextureHandle& value) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mTextureProperties.find(name) != Material::defaultMaterial->mTextureProperties.end());
    mTextureProperties[name] = value;
}
TextureHandle Material::getTextureProperty(const std::string& name) {
    assert(Material::defaultMaterial && Material::defaultMaterial->mTextureProperties.find(name) != Material::defaultMaterial->mTextureProperties.end());
    if(mTextureProperties.find(name) != mTextureProperties.end())
        return mTextureProperties[name];
    return Material::defaultMaterial->mTextureProperties[name];
}

void Material::RegisterFloatProperty(const std::string& name, float defaultValue) {
    Material::defaultMaterial->mFloatProperties[name] = defaultValue;
}
void Material::RegisterIntProperty(const std::string& name, int defaultValue) {
    Material::defaultMaterial->mIntProperties[name] = defaultValue;
}
void Material::RegisterVec4Property(const std::string& name, const glm::vec4& defaultValue) {
    Material::defaultMaterial->mVec4Properties[name] = defaultValue;
}
void Material::RegisterVec2Property(const std::string& name, const glm::vec2& defaultValue) {
    Material::defaultMaterial->mVec2Properties[name] = defaultValue;
}
void Material::RegisterTextureHandleProperty(const std::string& name, const TextureHandle& defaultValue) {
    Material::defaultMaterial->mTextureProperties[name] = defaultValue;
}

void Material::Init() {
    if(Material::defaultMaterial) return;
    Material::defaultMaterial = new Material {};
}

void Material::Clear() {
    if(!Material::defaultMaterial){ 
        return;
    }
    delete Material::defaultMaterial;
    Material::defaultMaterial = nullptr;
}

void Material::destroyResource() { 
    releaseResource();
}

void Material::releaseResource() { 
    mFloatProperties.clear();
    mIntProperties.clear();
    mVec2Properties.clear();
    mVec4Properties.clear();
    mTextureProperties.clear();
}
