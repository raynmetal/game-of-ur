/**
 *
 * @ingroup UrGameVisualLayer
 * @file ur_volume_control
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the definition of the class representing and controlling the 3D scene in which the game is played.
 * @version 0.3.2
 * @date 2026-09-24
 *
 */

#ifndef ZOAPPVOLUMECONTROL_H
#define ZOAPPVOLUMECONTROL_H

#include <toymaker/engine/sim_system.hpp>

/**
 * @ingroup UrGameVisualLayer
 * @brief The aspect class responsible for connecting the volume control UI in the scene with
 * Ur's sound player.
 *
 */
class UrUIVolumeControl: public ToyMaker::SimObjectAspect<UrUIVolumeControl> {
public:
    UrUIVolumeControl(): ToyMaker::SimObjectAspect<UrUIVolumeControl>{ 0 } {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUIVolumeControl"; }
    static std::shared_ptr<ToyMaker::BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

private:
    std::string mPathVolumeUp { "" };
    std::string mPathVolumeDown { "" };
    std::string mPathVolume { "" };

    void onButtonClicked(const std::string& button);
    void onButtonHoveredOver(const std::string& button);
    void updateControls();

    void onActivated() override;

public:
    ToyMaker::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
    ToyMaker::SignalObserver<const std::string&> mObserveButtonHoveredOver {
        *this, "ButtonHoveredOverObserved",
        [this](const std::string& button) { this->onButtonHoveredOver(button); }
    };
};

#endif

