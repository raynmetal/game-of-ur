#include "ui_button.hpp"
#include "ur_scene_manager.hpp"
#include "ur_records.hpp"

#include "ur_ui_records_browser.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIRecordsBrowser::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUIRecordsBrowser> recordsBrowser { std::make_shared<UrUIRecordsBrowser>() };
    return recordsBrowser;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIRecordsBrowser::clone() const {
    std::shared_ptr<UrUIRecordsBrowser> recordsBrowser { std::make_shared<UrUIRecordsBrowser>() };
    return recordsBrowser;
}

void UrUIRecordsBrowser::onActivated() {
    refreshRecords();
    openPage(mPage);
}

void UrUIRecordsBrowser::onButtonClicked(const std::string& button) {
    if(button == "next") {
        openPage(mPage + 1);

    }
    else if(button == "previous") {
        openPage(mPage - 1);

    } else {
        std::cout << "Button " << button << " was pressed!\n";
        /** TODO: open the details page for the record that was clicked */
    }
}

void UrUIRecordsBrowser::openPage(uint32_t page) {
    assert(hasPage(page) && "No such page exists");
    mPage = page;

    const std::size_t baseEntryIndex { mFetchedRecords.size() - page*5 };
    auto entry { mFetchedRecords.rbegin() + 5*page };
    auto end { mFetchedRecords.rend() };
    for(uint32_t entryIndex{0}; entryIndex < 5; ++entryIndex) {
        UIButton& recordButton { getSimObject().getByPath<UIButton&>(
                "/viewport_UI/record_" 
                + std::to_string(entryIndex)
                + "/@UIButton"
            )
        };
        if(entry != end) {
            recordButton.updateText(
                std::to_string(baseEntryIndex - entryIndex) + ". "
                + std::string{"Player A: "}
                + std::to_string(static_cast<int>(entry->mPlayerA.mCounters))
                + (entry->mPlayerA.mIsWinner? " W || ": " L || ")
                + std::string { "Player B: "}
                + std::to_string(static_cast<int>(entry->mPlayerB.mCounters))
                + (entry->mPlayerB.mIsWinner? " W": " L")
            );
            recordButton.enableButton();
            ++entry;
        } else {
            recordButton.updateText("NA");
            recordButton.disableButton();
        }
    }
    
    UIButton& next {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/next/@UIButton"
        )
    };
    UIButton& prev {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/previous/@UIButton"
        )
    };

    if(hasPage(page+1)) {
        next.enableButton();
    } else {
        next.disableButton();
    }

    if(hasPage(page-1)) {
        prev.enableButton();
    } else {
        prev.disableButton();
    }

}

bool UrUIRecordsBrowser::hasPage(uint32_t page) const {
    const std::size_t totalPages{ 1 + mFetchedRecords.size()/5 };
    return page < totalPages;
}

void UrUIRecordsBrowser::refreshRecords() {
    mFetchedRecords = getSimObject().getWorld().lock()
        ->getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<UrRecords&>(
            "/ur_records/@UrRecords"
        ).getAllRecords();
}
