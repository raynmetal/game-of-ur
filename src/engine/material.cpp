#include <vector>

#include "shader_program_manager.hpp"
#include "texture_manager.hpp"

#include "material.hpp"

Material::Material(const std::vector<TextureHandle>& textureHandles, float specularExponent) 
: 
    mTextureHandles{textureHandles},
    mSpecularExponent{specularExponent}
{}

void Material::bind(ShaderProgramHandle shaderProgramHandle) {
    //Set texture-related uniforms
    bool usingAlbedoMap {false};
    bool usingNormalMap {false};
    bool usingSpecularMap {false};

    GLuint textureUnit {0};
    for(const TextureHandle& textureHandle : mTextureHandles) {
        textureHandle.getResource().bind(textureUnit);
        //TODO: allow multiple materials to make up a single mesh
        switch(textureHandle.getResource().getUsage()) {
            case Texture::Albedo:
                usingAlbedoMap = true;
                shaderProgramHandle.getResource().setUInt("uMaterial.mTextureAlbedo", textureUnit);
            break;
            case Texture::Normal:
                usingNormalMap = true;
                shaderProgramHandle.getResource().setUInt("uMaterial.mTextureNormal", textureUnit);
            break;
            case Texture::Specular:
                usingSpecularMap = true;
                shaderProgramHandle.getResource().setUInt("uMaterial.mTextureSpecular", textureUnit);
            break;
            default: break;
        }
        ++textureUnit;
    }
    glActiveTexture(GL_TEXTURE0);

    shaderProgramHandle.getResource().setUBool("uMaterial.mUsingAlbedoMap", usingAlbedoMap);
    shaderProgramHandle.getResource().setUBool("uMaterial.mUsingNormalMap", usingNormalMap);
    shaderProgramHandle.getResource().setUBool("uMaterial.mUsingSpecularMap", usingSpecularMap);

    shaderProgramHandle.getResource().setUFloat("uMaterial.mSpecularExponent", mSpecularExponent);
}
