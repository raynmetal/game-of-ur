/**
 * @ingroup UrGameControlLayer
 * @file ur_records.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains aspect class definition for the records save system.
 * @version 0.3.2
 * @date 2025-09-14
 * 
 * 
 */

#ifndef ZOAPPRECORDS_H
#define ZOAPPRECORDS_H

#include <toymaker/engine/sim_system.hpp>

#include "game_of_ur_data/model.hpp"
#include "game_of_ur_data/serialize.hpp"


/**
 * @ingroup UrGameDataModel UrGameControlLayer
 * @brief The details of a single completed game.
 * 
 */
struct GameRecord {
    GameScoreData mSummary;
    PlayerData mPlayerA;
    PlayerData mPlayerB;
};

/**
 * @ingroup UrGameControlLayer
 * @brief Class responsible for loading, validating, and storing records of all completed games played on this platform.
 * 
 */
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
