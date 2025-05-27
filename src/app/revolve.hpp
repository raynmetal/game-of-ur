#ifndef ZOAPPREVOLVE_H
#define ZOAPPREVOLVE_H

#include "../engine/scene_system.hpp"
#include "../engine/sim_system.hpp"

class Revolve: public ToyMakersEngine::SimObjectAspect<Revolve>{
public:
    inline static std::string getSimObjectAspectTypeName() { return "Revolve"; }
    static std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    void variableUpdate(uint32_t variableStepMillis) override;

    std::shared_ptr<BaseSimObjectAspect> clone() const override;
private:
    Revolve (): SimObjectAspect<Revolve>{0} {}
};

#endif
