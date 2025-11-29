/**
 * @ingroup UrGameInteractionLayer
 * @file board_locations.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the class responsible for mapping locations on the 3D game board model to their equivalent coordinates in the data model.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPBOARDLOCATIONS_H
#define ZOAPPBOARDLOCATIONS_H

#include <toymaker/engine/sim_system.hpp>
#include <toymaker/builtins/interface_pointer_callback.hpp>

#include "game_of_ur_data/house.hpp"


/**
 * @ingroup UrGameInteractionLayer
 * @brief The aspect responsible for mapping points on the 3D game board to their equivalent coordinates on the game board data model.
 * 
 */
class BoardLocations: public ToyMaker::SimObjectAspect<BoardLocations>, public ToyMaker::ILeftClickable {
public:
    /**
     * @brief Gets the aspect type string associated with this class.
     * 
     * @return std::string This class' aspect type string.
     */
    inline static std::string getSimObjectAspectTypeName() { return "BoardLocations"; }

    /**
     * @brief Constructs this aspect from its JSON description.
     * 
     * This is its appearance in JSON:
     * 
     * ```jsonc
     * 
     * { "type": "BoardLocations" }
     * 
     * ```
     * 
     * @param jsonAspectProperties This aspect's description in JSON.
     * 
     * @return std::shared_ptr<BaseSimObjectAspect> The newly constructed aspect.
     */
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    /**
     * @brief Uses this aspect's data to construct a new aspect.
     * 
     * @return std::shared_ptr<BaseSimObjectAspect> The newly constructed aspect.
     */
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    /**
     * @brief Responds to left click events by logging them to the console, translating the location of the click to their game board grid coordinates.
     * 
     * @param clickLocation The 3D coordinates of the location on the board that was clicked.
     * @retval true The location clicked corresponded to a real game board grid location.
     * @retval false The location clicked did not correspond to a real game board grid location.
     */
    bool onPointerLeftClick(glm::vec4 clickLocation)  override;

    /**
     * @brief Responds to a full left click by emitting mSigBoardClicked with the clicked location's equivalent game board coordinates, if possible.
     * 
     * @param clickLocation The 3D coordinates of the point on the board that was clicked.
     * @retval true The clicked location caused mSigBoardClicked to be emitted.
     * @retval false No signal was emitted as a result of this click.
     */
    inline bool onPointerLeftRelease(glm::vec4 clickLocation) override { (void)clickLocation;/*prevent unused parameter warnings*/ return false; }

    /**
     * @brief The event emitted signalling to the 3D viewport controller that a location on the board was clicked.
     * 
     */
    ToyMaker::Signal<glm::u8vec2> mSigBoardClicked { *this, "BoardClicked" };

    /**
     * @brief Given the 2D coordinates of a point relative to the top surface of the board, returns the equivalent game board data model grid coordinates.
     * 
     * @param point The 2D coordinates of a point on the surface of the board.
     * @return glm::uvec2 The game board grid coordinates.
     */
    glm::uvec2 boardPointToGridIndices (glm::vec2 point) const;

    /**
     * @brief Translates game board data model grid coordinates to their real world 3D coordinates equivalent.
     * 
     * @param gridIndices The game board data model grid location.
     * @return glm::vec4 The equivalent 3D coordinates.
     */
    glm::vec4 gridIndicesToBoardPoint(glm::u8vec2 gridIndices) const;

private:
    /**
     * @brief Constructs a new Board Locations object.
     * 
     */
    BoardLocations(): SimObjectAspect<BoardLocations>{0} {}

    /**
     * @brief The lengths of the 3 rows of the game board representing the number of valid houses on it.
     * 
     */
    std::array<uint8_t, 3> mRowLengths {4, 12, 4};
};

#endif
