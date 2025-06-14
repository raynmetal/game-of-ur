#ifndef ZOAPPCLICKTOROLL_H
#define ZOAPPCLICKTOROLL_H

#include <random>

#include "interface_pointer_callback.hpp"
#include "../engine/sim_system.hpp"

class ClickToRoll: public ToyMakersEngine::SimObjectAspect<ClickToRoll>, public ILeftClickable, public IRightClickable {
public:
    enum class RollType: uint8_t {
        INITIATIVE,
        TURN
    };

    inline static std::string getSimObjectAspectTypeName() { return "ClickToRoll"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onPointerLeftClick(glm::vec4 clickLocation)  override;
    bool onPointerRightClick(glm::vec4 clickLocation) override;

    inline bool getYesNoDieState() const { return mYesNoDie; }
    uint8_t getPrimaryDieState() const { return mPrimaryDie; }
    uint8_t getResultScore() const;
protected:

private:
    ClickToRoll(): SimObjectAspect<ClickToRoll>{0} {}

    void rollPrimary();
    void rollYesNo();

    void setResultText();

    RollType mRollType{ RollType::INITIATIVE };
    bool mYesNoDie { false };
    uint8_t mPrimaryDie { 1 };

    std::random_device mRandomDevice {};
    std::default_random_engine mRandomEngine { mRandomDevice() };
    std::uniform_int_distribution<int> mPrimaryDieDistribution { 1, 4 };
    std::uniform_int_distribution<int> mYesNoDieDistribution { 0, 1 };
};

#endif
