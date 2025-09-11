#ifndef ZOAPPPLAYERCPURANDOM_H
#define ZOAPPPLAYERCPURANDOM_H

#include <random>

#include "toymaker/sim_system.hpp"

#include "ur_controller.hpp"

class PlayerCPURandom: public ToyMaker::SimObjectAspect<PlayerCPURandom> {
public:
    PlayerCPURandom(): SimObjectAspect<PlayerCPURandom>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrPlayerCPURandom"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

private:
    std::string mControllerPath {};
    std::unique_ptr<UrPlayerControls> mControls {};

    std::random_device mRandomDevice {};
    std::default_random_engine mRandomEngine { mRandomDevice() };

    void onActivated() override;
    void onMovePrompted(GamePhaseData phaseData);

    ToyMaker::SignalObserver<GamePhaseData> mObserveMovePrompted { *this, "MovePromptedObserved", [this](GamePhaseData phaseData) { this->onMovePrompted(phaseData); }};
public:
};

#endif
