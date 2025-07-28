#include "ur_records.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrRecords::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrRecords> records { std::make_shared<UrRecords>() };
    return records;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrRecords::clone() const {
    std::shared_ptr<UrRecords> records { std::make_shared<UrRecords>() };
    records->mLoadedRecords = mLoadedRecords;
    return records;
}

void UrRecords::ApplyInvariants(const GameRecord& gameRecord) {
    return;
}

void UrRecords::submitRecord(const GameRecord& gameRecord) {
    ApplyInvariants(gameRecord);
    mLoadedRecords.push_back(gameRecord);
}

void UrRecords::onActivated() {
    std::cout << "Ur Records has been loaded successfully!\n";
}

void from_json(const nlohmann::json& json, GameRecord& gameRecord) {
    json.at("summary").get_to(gameRecord.mSummary);
    json.at("player_a").get_to(gameRecord.mPlayerA);
    json.at("player_b").get_to(gameRecord.mPlayerB);
}

void to_json(nlohmann::json& json, const GameRecord& gameRecord) {
    json = {
        {"summary", gameRecord.mSummary},
        {"player_a", gameRecord.mPlayerA},
        {"player_b", gameRecord.mPlayerB},
    };
}
