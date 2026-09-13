#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>

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

bool BoardLocations::onPointerHover(glm::vec4 hoverLocation) {
    std::cout << "Board hover: " << glm::to_string(hoverLocation) << "\n";
    if(hoverLocation.y >= -0.1){
        const glm::u8vec2 boardLocation {
            boardPointToGridIndices({ hoverLocation.x, hoverLocation.z })
        };
        if(boardLocation.x >= mRowLengths.size() || boardLocation.y >= mRowLengths[boardLocation.x]) {
            return true;
        }
        mSigBoardHovered.emit(boardLocation);
    }
    return true;
}

bool BoardLocations::onPointerLeave() {
    return true;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> BoardLocations::create(const nlohmann::json& jsonAspectProperties) {
    (void)jsonAspectProperties; // prevent unused parameter warnings
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> BoardLocations::clone() const {
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

glm::uvec2 BoardLocations::boardPointToGridIndices(glm::vec2 point) const {
    const ToyMaker::AxisAlignedBounds::Extents boardExtents { getComponent<ToyMaker::AxisAlignedBounds>().getAxisAlignedBoxExtents() };
    glm::vec2 normalizedPoint {
        glm::clamp(
            (point.x - boardExtents.second.x) / (boardExtents.first.x - boardExtents.second.x),
            0.f, .99999f
        ),
        glm::clamp(
            (point.y - boardExtents.second.z) / (boardExtents.first.z - boardExtents.second.z),
            0.f, .99999f
        ),
    };
    return {normalizedPoint.x * mRowLengths.size(), normalizedPoint.y * mRowLengths[1]};
}

glm::vec4 BoardLocations::gridIndicesToBoardPoint(glm::u8vec2 gridIndices) const {
    const ToyMaker::AxisAlignedBounds::Extents boardExtents {
        getComponent<ToyMaker::AxisAlignedBounds>().getAxisAlignedBoxExtents()
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
