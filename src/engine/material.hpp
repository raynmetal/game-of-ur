#ifndef ZOMATERIAL_H
#define ZOMATERIAL_H

#include <vector>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "shader_program_manager.hpp"

#include "texture_manager.hpp"

#include "resource_manager.hpp"

class Material : IResource {
public:
    Material() = default;
    Material(const std::vector<TextureHandle>& textureHandles, const float specularExponent);
    void bind(ShaderProgramHandle shaderProgramHandle);
private:
    std::vector<TextureHandle> mTextureHandles {};
    float mSpecularExponent{18.f};

    void destroyResource();
    void releaseResource();
friend class ResourceManager<Material>;
};

#endif
