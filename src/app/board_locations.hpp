#ifndef ZOAPPBOARDLOCATIONS_H
#define ZOAPPBOARDLOCATIONS_H

#include "../engine/sim_system.hpp"

#include "interface_pointer_callback.hpp"
#include "game_of_ur_data/house.hpp"


class BoardLocations: public ToyMakersEngine::SimObjectAspect<BoardLocations>, public ILeftClickable {
public:
    inline static std::string getSimObjectAspectTypeName() { return "BoardLocations"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onPointerLeftClick(glm::vec4 clickLocation)  override;
    inline bool onPointerLeftRelease(glm::vec4 clickLocation) override { return false; }

    ToyMakersEngine::Signal<glm::u8vec2> mSigBoardClicked { *this, "BoardClicked" };

    glm::uvec2 boardPointToGridIndices (glm::vec2 point) const;
    glm::vec4 gridIndicesToBoardPoint(glm::u8vec2 gridIndices) const;

private:
    BoardLocations(): SimObjectAspect<BoardLocations>{0} {}

    std::array<uint8_t, 3> mRowLengths {4, 12, 4};
};

#endif
