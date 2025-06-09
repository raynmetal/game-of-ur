#ifndef ZOAPPCLICKCALLBACKIMPL1
#define ZOAPPCLICKCALLBACKIMPL1

#include "interface_pointer_callback.hpp"
#include "../engine/sim_system.hpp"

class ClickCallbackImpl1: public ToyMakersEngine::SimObjectAspect<ClickCallbackImpl1>, public ILeftClickable {
public:
    inline static std::string getSimObjectAspectTypeName() { return "ClickCallbackImpl1"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onPointerLeftClick()  override;

protected:

private:
    ClickCallbackImpl1(): SimObjectAspect<ClickCallbackImpl1>{0} {}
};

#endif
