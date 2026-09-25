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
    mSounds[static_cast<uint8_t>(UrSoundFX::MUSIC)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Background_Music_Sound");
    mSounds[static_cast<uint8_t>(UrSoundFX::BUTTON_CLICK)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Click_Sound");
    mSounds[static_cast<uint8_t>(UrSoundFX::BUTTON_HOVER)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Hover_Sound");
    mSounds[static_cast<uint8_t>(UrSoundFX::DICE_ROLL)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Dice_Roll_Sound");
    mSounds[static_cast<uint8_t>(UrSoundFX::PIECE_MOVE)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Piece_Drag_Sound");
    mSounds[static_cast<uint8_t>(UrSoundFX::PIECE_LAUNCH)] = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Piece_Launch_Sound");

    mChannelMusic = getSimObject().getWorld().lock()->getSystem<ToyMaker::SoundSystem>()->createChannel();
    mChannelEffects = getSimObject().getWorld().lock()->getSystem<ToyMaker::SoundSystem>()->createChannel();

    mChannelEffects->setLoopCount(0);
    mChannelMusic->setSound(*mSounds[static_cast<uint8_t>(UrSoundFX::MUSIC)]);
    mChannelMusic->setLoopCount(-1);
    mChannelMusic->play();
    assert(mChannelMusic->isPlaying() && "Could not start background music playback");

    std::cout << "Ur Sound: sounds loaded successfully!\n";
}

void UrSoundPlayer::playEffect(UrSoundFX effect, uint8_t priority) {
    assert(effect != UrSoundFX::TOTAL && effect != UrSoundFX::MUSIC && "Invalid effect requested");
    assert(mChannelMusic->isPlaying() && "Music should always be playing");

    // guard: avoid interrupting if another effect is currently playing with
    // a higher priority
    if(priority < mEffectPriority && mChannelEffects->isPlaying()) {
        return;
    }

    mChannelEffects->stop(0);
    mChannelEffects->setSound(*mSounds[static_cast<uint8_t>(effect)]);
    mEffectPriority = priority;
    mChannelEffects->play();
}

void UrSoundPlayer::incrementVolume5() {
    if(mVolume >= 100) {
        return;
    }

    mVolume = glm::min(mVolume + 5, 100);
    const float volume { mVolume / 100.f };

    mChannelMusic->setGain(volume);
    mChannelEffects->setGain(volume);
}

void UrSoundPlayer::decrementVolume5() {
    if(mVolume <= 0) {
        return;
    }

    mVolume = glm::max(static_cast<int>(mVolume) - 5, 0);
    const float volume { mVolume / 100.f };

    mChannelMusic->setGain(volume);
    mChannelEffects->setGain(volume);
}
