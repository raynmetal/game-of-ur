#include <glm/gtx/string_cast.hpp>

#include "board_locations.hpp"

uint8_t BoardLocation::getRow() const {
    return (mDescription&kRowMask) >> kRowOffset;
}

void BoardLocation::setRow(uint8_t row) {
    assert (row < 3 && "Row must be a number in range 0-2");
    mDescription = (mDescription & (~kRowMask)) | ((row << kRowOffset) & kRowMask);
}

uint8_t BoardLocation::getCol() const {
    return (mDescription&kColMask) >> kColOffset;
}

void BoardLocation::setCol(uint8_t col) {
    assert(col < 12 && "Col must be a number in range 0-11");
    mDescription = (mDescription & (~kColMask)) | ((col << kColOffset) & kColMask);
}

BoardLocation::CellType BoardLocation::getType() const {
    return static_cast<CellType>(mDescription&kCellTypeMask);
}

void BoardLocation::setType(BoardLocation::CellType type) {
    mDescription = (
        (mDescription & (~kCellTypeMask))
        | (static_cast<LocationDescription>(type) & kCellTypeMask)
    );
}

bool BoardLocations::onPointerLeftClick(glm::vec4 clickLocation) {
    std::cout << "Left click: " << glm::to_string(clickLocation) << "\n";
    if(clickLocation.y >= -std::numeric_limits<float>::epsilon()) {
        glm::uvec2 gridLocation { boardPointToGridIndices({clickLocation.x, clickLocation.z}) };
        const BoardLocation boardLocation { mGrid[gridLocation.x][gridLocation.y] };
        std::cout << "\tcell type: ";
        switch(boardLocation.getType()) {
            case BoardLocation::CellType::BATTLEFIELD:
                std::cout << "battlefield\n";
                break;
            case BoardLocation::CellType::INVALID:
                std::cout << "invalid\n";
                break;
            case BoardLocation::CellType::PLAYER_ONE:
                std::cout << "player one\n";
                break;
            case BoardLocation::CellType::PLAYER_TWO:
                std::cout << "player two\n";
                break;
        }
        std::cout << "\trow: " << static_cast<int>(boardLocation.getRow()) << "\n";
        std::cout << "\tcol: " << static_cast<int>(boardLocation.getCol()) << "\n";
    }
    return true;
}

bool BoardLocations::onPointerRightClick(glm::vec4 clickLocation) {
    std::cout << "Right click: " << glm::to_string(clickLocation) << "\n";
    return true;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BoardLocations::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BoardLocations::clone() const {
    return std::shared_ptr<BoardLocations>{ new BoardLocations{} };
}

glm::uvec2 BoardLocations::boardPointToGridIndices (glm::vec2 point) const {
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
    return {normalizedPoint.x * mGrid.size(), normalizedPoint.y * mGrid[0].size()};
}
