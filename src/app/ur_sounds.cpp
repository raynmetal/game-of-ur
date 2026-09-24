#include <iostream>

#include <toymaker/engine/sound/system.hpp>

#include "ur_sounds.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSoundPlayer::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSoundPlayer> sounds { std::make_shared<UrSoundPlayer>() };
    return sounds;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSoundPlayer::clone() const {
    std::shared_ptr<UrSoundPlayer> sounds { std::make_shared<UrSoundPlayer>() };
    return sounds;
}


void UrSoundPlayer::onActivated() {
    mSounds[UrSoundFX::MUSIC] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Background_Music_Sound");
    mSounds[UrSoundFX::BUTTON_CLICK] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Click_Sound");
    mSounds[UrSoundFX::BUTTON_HOVER] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Hover_Sound");
    mSounds[UrSoundFX::DICE_ROLL] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Dice_Roll_Sound");
    mSounds[UrSoundFX::PIECE_MOVE] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Piece_Drag_Sound");
    mSounds[UrSoundFX::PIECE_LAUNCH] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Piece_Launch_Sound");

    mChannelMusic = getSimObject().getWorld().lock()->getSystem<ToyMaker::SoundSystem>()->createChannel();
    mChannelEffects = getSimObject().getWorld().lock()->getSystem<ToyMaker::SoundSystem>()->createChannel();

    std::cout << "Ur Sound: sounds loaded successfully!\n";
}
