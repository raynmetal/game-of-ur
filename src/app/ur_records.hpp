#ifndef ZOAPPRECORDS_H
#define ZOAPPRECORDS_H

#include "toymaker/sim_system.hpp"

#include "game_of_ur_data/model.hpp"
#include "game_of_ur_data/serialize.hpp"


struct GameRecord {
    GameScoreData mSummary;
    PlayerData mPlayerA;
    PlayerData mPlayerB;
};

class UrRecords: public ToyMaker::SimObjectAspect<UrRecords> {
public:
    UrRecords(): SimObjectAspect<UrRecords>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrRecords"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void submitRecord(const GameRecord& gameRecord);

    inline GameRecord getLatestRecord() const { return mLoadedRecords.back(); }
    inline std::vector<GameRecord> getAllRecords() const { return mLoadedRecords; }

private:
    static void ApplyInvariants(const GameRecord& gameRecord);
    void onActivated() override;
    void onDeactivated() override;

    std::vector<GameRecord> mLoadedRecords {};
    std::string mRecordsPath {};
};

void from_json(const nlohmann::json& json, GameRecord& gameRecord);
void to_json(nlohmann::json& json, const GameRecord& gameRecord);

#endif
