#ifndef ZOAPPPOINTERCALLBACK_H
#define ZOAPPPOINTERCALLBACK_H

#include "../engine/sim_system.hpp"

class PointerCallback: public SimObjectAspect<PointerCallback> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "PointerCallback"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onClick();
protected:
private:
    PointerCallback(): SimObjectAspect<PointerCallback>{0} {}
};

#endif
