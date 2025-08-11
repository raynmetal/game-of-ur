#include <filesystem>
#include <fstream>

#include "nlohmann/json.hpp"
#include "ur_records.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrRecords::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrRecords> records { std::make_shared<UrRecords>() };
    records->mRecordsPath = jsonAspectProperties.at("records_path").get<std::string>();
    return records;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrRecords::clone() const {
    std::shared_ptr<UrRecords> records { std::make_shared<UrRecords>() };
    records->mLoadedRecords = mLoadedRecords;
    records->mRecordsPath = mRecordsPath;
    return records;
}

void UrRecords::ApplyInvariants(const GameRecord& gameRecord) {
    assert(gameRecord.mSummary.mCommonPoolCounters == 0 && "There should be no counters in the local pool at the \
        end of a game");
    assert(
        ((gameRecord.mSummary.mPlayerOneCounters + gameRecord.mSummary.mPlayerTwoCounters) == 50)
        && "There should be a total of 50 counters distributed amongst the players at the end of the game"
    );
    assert(
        (
            (
                gameRecord.mSummary.mPlayerOneVictoryPieces == 5
                && gameRecord.mSummary.mPlayerTwoVictoryPieces < 5
            ) || (
                gameRecord.mSummary.mPlayerTwoVictoryPieces == 5
                && gameRecord.mSummary.mPlayerOneVictoryPieces < 5
            )
        ) && "Only one player should have had all 5 of their pieces reach the end of the board"
    );
    assert(
        (
            gameRecord.mSummary.mPlayerOneCounters == (
                (gameRecord.mPlayerA.mRole == RoleID::BLACK)?
                gameRecord.mPlayerA.mCounters:
                gameRecord.mPlayerB.mCounters
            ) && gameRecord.mSummary.mPlayerTwoCounters == (
                (gameRecord.mPlayerA.mRole == RoleID::WHITE)?
                gameRecord.mPlayerA.mCounters:
                gameRecord.mPlayerB.mCounters
            )
        ) && "Game record summary counters should correspond to detailed game counters"
    );
    assert(
        (
            (gameRecord.mPlayerA.mIsWinner  && !gameRecord.mPlayerB.mIsWinner)
            || (gameRecord.mPlayerB.mIsWinner && !gameRecord.mPlayerA.mIsWinner)
        ) && "There can only be one winner according to the detailed game summary"
    );
    return;
}

void UrRecords::submitRecord(const GameRecord& gameRecord) {
    ApplyInvariants(gameRecord);
    mLoadedRecords.push_back(gameRecord);
    std::cout << "Ur Records: current records: \n" << nlohmann::to_string(nlohmann::json{mLoadedRecords}) << "\n";
}

void UrRecords::onActivated() {
    if(!std::filesystem::exists(mRecordsPath)) { return; }

    std::ifstream jsonFileStream;
    jsonFileStream.open(mRecordsPath);
    nlohmann::json recordsJSON = nlohmann::json::parse(jsonFileStream);
    jsonFileStream.close();

    for(
        auto record {recordsJSON.cbegin()}, end{recordsJSON.cend()};
        record != end;
        record++
    ) {
        submitRecord(*record);
    }

    std::cout << "Ur Records: records loaded successfully!\n";
}

void UrRecords::onDeactivated() {
    std::ofstream jsonFileStream;
    jsonFileStream.open(mRecordsPath);
    const nlohmann::json recordsJson = mLoadedRecords;
    const std::string recordsSerialized { recordsJson.dump() };
    jsonFileStream.write(recordsSerialized.c_str(), recordsSerialized.size());
    jsonFileStream.close();

    std::cout << "Ur Records: records saved successfully\n";
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
