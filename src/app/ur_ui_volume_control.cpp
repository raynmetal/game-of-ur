#include <toymaker/builtins/ui_button.hpp>
#include <toymaker/builtins/ui_text.hpp>

#include "ur_sounds.hpp"
#include "ur_ui_volume_control.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIVolumeControl::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUIVolumeControl> volumeControl { std::make_shared<UrUIVolumeControl>() };
    volumeControl->mPathVolumeUp = jsonAspectProperties.at("volume_up");
    volumeControl->mPathVolumeDown = jsonAspectProperties.at("volume_down");
    volumeControl->mPathVolume = jsonAspectProperties.at("volume");
    return volumeControl;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIVolumeControl::clone() const {
    std::shared_ptr<UrUIVolumeControl> volumeControl { std::make_shared<UrUIVolumeControl>() };
    volumeControl->mPathVolumeUp = mPathVolumeUp;
    volumeControl->mPathVolumeDown = mPathVolumeDown;
    volumeControl->mPathVolume = mPathVolume;
    return volumeControl;
}

void UrUIVolumeControl::onActivated() {
    std::shared_ptr<ToyMaker::SimObject> volumeUp {
        std::static_pointer_cast<ToyMaker::SimObject>(getSimObject().getNode(mPathVolumeUp))
    };
    std::shared_ptr<ToyMaker::SimObject> volumeDown {
        std::static_pointer_cast<ToyMaker::SimObject>(getSimObject().getNode(mPathVolumeDown))
    };

    connect("ButtonReleased", "ButtonClickedObserved", volumeUp->getAspect<ToyMaker::UIButton>());
    connect("ButtonReleased", "ButtonClickedObserved", volumeDown->getAspect<ToyMaker::UIButton>());

    connect("ButtonHoveredOver", "ButtonHoveredOverObserved", volumeUp->getAspect<ToyMaker::UIButton>());
    connect("ButtonHoveredOver", "ButtonHoveredOverObserved", volumeDown->getAspect<ToyMaker::UIButton>());

    updateControls();
}

void UrUIVolumeControl::updateControls() {
    std::shared_ptr<ToyMaker::SimObject> volumeUp {
        std::static_pointer_cast<ToyMaker::SimObject>(getSimObject().getNode(mPathVolumeUp))
    };
    std::shared_ptr<ToyMaker::SimObject> volumeDown {
        std::static_pointer_cast<ToyMaker::SimObject>(getSimObject().getNode(mPathVolumeDown))
    };
    std::shared_ptr<ToyMaker::SimObject> volume {
        std::static_pointer_cast<ToyMaker::SimObject>(getSimObject().getNode(mPathVolume))
    };

    const auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    volume->getAspect<ToyMaker::UIText>().updateText("vol: " + std::to_string(static_cast<int>(soundPlayer.getVolume())) );

    volumeUp->getAspect<ToyMaker::UIButton>().disableButton();
    volumeDown->getAspect<ToyMaker::UIButton>().disableButton();
    if(soundPlayer.getVolume() < 100) {
        volumeUp->getAspect<ToyMaker::UIButton>().enableButton();
    }
    if(soundPlayer.getVolume() > 0) {
        volumeDown->getAspect<ToyMaker::UIButton>().enableButton();
    }
}

void UrUIVolumeControl::onButtonClicked(const std::string& button) {
    assert((button == "+volume" || button == "-volume") && "Invalid button event received");
    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };

    if(button == "+volume") {
        soundPlayer.incrementVolume5();
    } else {
        soundPlayer.decrementVolume5();
    }

    soundPlayer.playEffect(UrSoundFX::BUTTON_CLICK, 1);
    updateControls();
}

void UrUIVolumeControl::onButtonHoveredOver(const std::string& button) {
    assert((button == "+volume" || button == "-volume") && "Invalid button event received");

    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    soundPlayer.playEffect(UrSoundFX::BUTTON_HOVER);
}

