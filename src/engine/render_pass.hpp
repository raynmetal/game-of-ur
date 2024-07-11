#ifndef ZORENDERPASS_H
#define ZORENDERPASS_H

#include <vector>
#include <map>
#include <string>
#include "render_stage.hpp"

class BaseRenderPass {
public:
    virtual void setup()=0;
    virtual void execute()=0;

    TextureHandle getRenderTargetTexture(const std::string& renderTargetName);
protected:
    std::vector<BaseOffscreenRenderStage> mRenderStages {};
    std::map<std::string, TextureHandle> mRenderTargets {};
};

class OpaqueRenderPass : public BaseRenderPass {
public:
    virtual void setup() override;
    virtual void execute() override;
};

#endif
