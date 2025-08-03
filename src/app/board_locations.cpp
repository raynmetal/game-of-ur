#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "board_locations.hpp"

bool BoardLocations::onPointerLeftClick(glm::vec4 clickLocation) {
    std::cout << "Left click: " << glm::to_string(clickLocation) << "\n";
    if(clickLocation.y >= -0.1) {
        const glm::u8vec2 boardLocation{ boardPointToGridIndices({clickLocation.x, clickLocation.z}) };
        if(boardLocation.x >= mRowLengths.size() || boardLocation.y >= mRowLengths[boardLocation.x]) {
            return false;
        }

        mSigBoardClicked.emit(boardLocation);
    }
    return true;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BoardLocations::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BoardLocations::clone() const {
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

glm::uvec2 BoardLocations::boardPointToGridIndices(glm::vec2 point) const {
    const ToyMakersEngine::AxisAlignedBounds::Extents boardExtents { getComponent<ToyMakersEngine::AxisAlignedBounds>().getAxisAlignedBoxExtents() };
    glm::vec2 normalizedPoint {
        (point.x - boardExtents.second.x) / (boardExtents.first.x - boardExtents.second.x),
        (point.y - boardExtents.second.z) / (boardExtents.first.z - boardExtents.second.z),
    };
    assert(
        normalizedPoint.x >= 0.f && normalizedPoint.x <= 1.f
        && normalizedPoint.y >= 0.f && normalizedPoint.y <= 1.f
        && "selected point is beyond the bounds of the board"
    );
    return {normalizedPoint.x * mRowLengths.size(), normalizedPoint.y * mRowLengths[1]};
}

glm::vec4 BoardLocations::gridIndicesToBoardPoint(glm::u8vec2 gridIndices) const {
    const ToyMakersEngine::AxisAlignedBounds::Extents boardExtents {
        getComponent<ToyMakersEngine::AxisAlignedBounds>().getAxisAlignedBoxExtents()
    };
    const glm::vec4 centerOffset {
        (boardExtents.first.x - boardExtents.second.x) / (2.f * mRowLengths.size()),
        0.f,
        (boardExtents.first.z - boardExtents.second.z) / (2.f * mRowLengths[1]),
        0.f,
    };
    const glm::vec4 cellSize { 2.f * centerOffset };
    const glm::vec4 newCoordinates {
        glm::vec4{boardExtents.second.x, 0.f, boardExtents.second.z, 1.f}
        + cellSize * glm::vec4{gridIndices.x, 0.f, gridIndices.y, 0.f}
        + centerOffset
    };
    return newCoordinates;
}
