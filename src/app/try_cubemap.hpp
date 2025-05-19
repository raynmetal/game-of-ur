#ifndef ZOTRYCUBEMAP_H
#define ZOTRYCUBEMAP_H

#include "../engine/sim_system.hpp"

class TryCubemap: public SimObjectAspect<TryCubemap> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "TryCubemap"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

private:
    TryCubemap(): SimObjectAspect<TryCubemap>{0} {}

    std::weak_ptr<Material> mMaterial {};
};

#endif
