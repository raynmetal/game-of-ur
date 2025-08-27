#ifndef ZOAPPUIVERSIONTEXT_H
#define ZOAPPUIVERSIONTEXT_H

#include "../engine/sim_system.hpp"

class UrUIVersion: public ToyMakersEngine::SimObjectAspect<UrUIVersion> {
public:
    UrUIVersion(): SimObjectAspect<UrUIVersion>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUIVersion"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
private:
};


#endif
