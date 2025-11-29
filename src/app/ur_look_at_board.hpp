/**
 * @ingroup UrGameVisualLayer
 * @file ur_look_at_board.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains a utility class for centering the game camera in the 3D scene on the game board object.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPLOOKATBOARD_H
#define ZOAPPLOOKATBOARD_H

#include "toymaker/sim_system.hpp"

/**
 * @ingroup UrGameVisualLayer
 * @brief Stupid utility class for centering the game camera in the 3D scene on the game board object.
 * 
 */
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
