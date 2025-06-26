#ifndef ZOAPPVALUEONCLICK_H
#define ZOAPPVALUEONCLICK_H

#include <random>

#include "interface_pointer_callback.hpp"
#include "../engine/signals.hpp"
#include "../engine/sim_system.hpp"

class ValueOnClick: public ToyMakersEngine::SimObjectAspect<ValueOnClick>, public ILeftClickable {
public:
    inline static std::string getSimObjectAspectTypeName() { return "ValueOnClick"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    bool onPointerLeftClick(glm::vec4 clickLocation)  override;

    ToyMakersEngine::Signal<const std::string&> mSigClicked { *this, "Clicked" };
private:
    ValueOnClick(): SimObjectAspect<ValueOnClick>{0} {}
    std::string mReturnValue { "" };
};

#endif
