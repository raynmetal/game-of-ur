#ifndef ZOAPPCLICKCALLBACKIMPL2
#define ZOAPPCLICKCALLBACKIMPL2

#include "interface_pointer_callback.hpp"
#include "../engine/sim_system.hpp"

class ClickCallbackImpl2: public SimObjectAspect<ClickCallbackImpl2>, public IClickable {
public:
    inline static std::string getSimObjectAspectTypeName() { return "ClickCallbackImpl2"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onPointerClick()  override;

protected:

private:
    ClickCallbackImpl2(): SimObjectAspect<ClickCallbackImpl2>{0} {}
};

#endif
