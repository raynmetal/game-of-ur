#ifndef ZOAPPLOOKATBOARD_H
#define ZOAPPLOOKATBOARD_H

#include "../engine/sim_system.hpp"

class UrLookAtBoard: public ToyMakersEngine::SimObjectAspect<UrLookAtBoard> {
public:
    UrLookAtBoard() : SimObjectAspect<UrLookAtBoard>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrLookAtBoard"; }
    static std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> clone() const override;

private:
    glm::vec3 mOffset { 0.f, 1.f, 2.f };
    void onActivated() override;
};


#endif
