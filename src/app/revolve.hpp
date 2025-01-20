#ifndef ZOAPPREVOLVE_H
#define ZOAPPREVOLVE_H

#include "../engine/scene_system.hpp"
#include "../engine/sim_system.hpp"

class Revolve: public SimObjectAspect<Revolve>{
public:
    inline static std::string getSimObjectAspectTypeName() { return "Revolve"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    void update(uint32_t deltaSimtimeMillis) override;

    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;

    void onSecondPassed();

    SignalObserver<> mSecondPassedObserved { *this, "secondPassedObserved", [this]() { this->onSecondPassed(); } };
private:
    Revolve (): SimObjectAspect<Revolve>{0} {}
};

#endif
