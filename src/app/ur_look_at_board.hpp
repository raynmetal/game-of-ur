#ifndef ZOAPPLOOKATBOARD_H
#define ZOAPPLOOKATBOARD_H

#include "toymaker/sim_system.hpp"

class UrLookAtBoard: public ToyMaker::SimObjectAspect<UrLookAtBoard> {
public:
    UrLookAtBoard() : SimObjectAspect<UrLookAtBoard>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrLookAtBoard"; }
    static std::shared_ptr<ToyMaker::BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<ToyMaker::BaseSimObjectAspect> clone() const override;

private:
    glm::vec3 mOffset { 0.f, 1.f, 2.f };
    void onActivated() override;
};


#endif
