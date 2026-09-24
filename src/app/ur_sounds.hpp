/**
 * @ingroup UrGameControlLayer
 * @file ur_records.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains aspect class definition for the records save system.
 * @version 0.3.2
 * @date 2025-09-14
 * 
 * 
 */

#ifndef ZOAPPSOUNDS_H
#define ZOAPPSOUNDS_H

#include <array>

#include <toymaker/engine/sound/types.hpp>
#include <toymaker/engine/sim_system.hpp>

enum class UrSoundFX: uint8_t {
    MUSIC,
    BUTTON_HOVER,
    BUTTON_CLICK,
    PIECE_LAUNCH,
    PIECE_MOVE,
    DICE_ROLL,

    TOTAL,
};

/**
 * @ingroup UrGameControlLayer
 * @brief Class responsible for loading, validating, and storing records of all completed games played on this platform.
 *
 */
class UrSoundPlayer: public ToyMaker::SimObjectAspect<UrSoundPlayer> {
public:
    UrSoundPlayer(): SimObjectAspect<UrSoundPlayer>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrSoundPlayer"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    void playEffect(UrSoundFX effect, uint8_t priority=0);

private:
    std::array<std::shared_ptr<ToyMaker::Sound>, static_cast<uint8_t>(UrSoundFX::TOTAL)> mSounds {};

    std::unique_ptr<ToyMaker::SoundChannel> mChannelMusic { nullptr };
    std::unique_ptr<ToyMaker::SoundChannel> mChannelEffects { nullptr };
    uint8_t mEffectPriority { 0 };

    void onActivated() override;
};

#endif
