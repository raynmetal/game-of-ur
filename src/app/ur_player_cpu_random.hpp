/**
 * @file ur_player_cpu_random.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the class definition of the CPU player controller.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPPLAYERCPURANDOM_H
#define ZOAPPPLAYERCPURANDOM_H

#include <random>

#include "toymaker/sim_system.hpp"

#include "ur_controller.hpp"

/**
 * @brief An aspect representing a computer controlled player of the game of ur, which makes its decisions completely randomly.
 * 
 */
class PlayerCPURandom: public ToyMaker::SimObjectAspect<PlayerCPURandom> {
public:
    PlayerCPURandom(): SimObjectAspect<PlayerCPURandom>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrPlayerCPURandom"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

private:
    /**
     * @brief The path to the game controller this player interfaces with.
     * 
     */
    std::string mControllerPath {};

    /**
     * @brief The controls object created by the game controller. 
     * 
     * Provides the interface through which this player is able to interact with the (data model representation of the) game.
     * 
     */
    std::unique_ptr<UrPlayerControls> mControls {};

    /**
     * @brief The device responsible for providing this class with random numbers.
     * 
     */
    std::random_device mRandomDevice {};

    /**
     * @brief An engine using the random device.
     * 
     */
    std::default_random_engine mRandomEngine { mRandomDevice() };

    /**
     * @brief Broadcasts its existence to UrController and receives in exchange an instance of UrPlayerControls.
     * 
     */
    void onActivated() override;

    /**
     * @brief Callback for an event from GameOfUrController, prompting this player for a new game-related action.
     * 
     * @param phaseData 
     */
    void onMovePrompted(GamePhaseData phaseData);

    /**
     * @brief The observer connected with this aspect, responsible for receiving and responding to move prompt events.
     * 
     */
    ToyMaker::SignalObserver<GamePhaseData> mObserveMovePrompted { *this, "MovePromptedObserved", [this](GamePhaseData phaseData) { this->onMovePrompted(phaseData); }};
public:
};

#endif
