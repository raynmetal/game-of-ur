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

