#include "Horizon.h"
#include "Maths.h"

namespace Horizon {

    namespace {
        float simulationTime;
        std::map<BWAPI::Unit, HorizonUnit> enemyUnits;
        std::map<BWAPI::Unit, HorizonUnit> myUnits;

        void simulate(HorizonOutput& newOutput, HorizonUnit& unit) {

            float enemyGrdSim;
            float enemyAirSim;
            float myGrdSim;
            float myAirSim;
            float unitSpeed = unit.getSpeed() * 24.0f;
            bool sync = false;

            const auto canAddToSim = [&](HorizonUnit& u) {
                if (!u.unit()
                    || u.getType().isWorker()
                    || (u.unit()->exists() && (u.unit()->isStasised() || u.unit()->isMorphing() || !u.unit()->isCompleted()))
                    || (u.getVisibleAirStrength() <= 0.0 && u.getVisibleGroundStrength() <= 0.0))
                    return false;
                return true;
            };

            const auto simEnemies = [&]() {
                for (auto &e : enemyUnits) {
                    auto &enemy = e.second;
                    if (!canAddToSim(enemy))
                        continue;

                    const auto enemyRange = unit.getType().isFlyer() ? enemy.getAirRange() : enemy.getGroundRange();
                    const auto widths = double(enemy.getType().width() + unit.getType().width()) / 2.0;
                    const auto distance = std::max(0.0, enemy.getPosition().getDistance(unit.getEngagePosition()) - enemyRange - widths);
                    const auto speed = enemy.getSpeed() > 0.0 ? 24.0 * enemy.getSpeed() : 24.0 * unit.getSpeed();
                    auto simRatio = simulationTime - (distance / speed);

                    // If the unit doesn't affect this simulation
                    if (simRatio <= 0.0
                        || (enemy.getSpeed() <= 0.0 && distance > 0.0)
                        || (enemy.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode && (enemy.getPosition().getDistance(unit.getPosition()) - widths) < 64.0))
                        continue;

                    // High ground bonus
                    if (!enemy.getType().isFlyer() && BWAPI::Broodwar->getGroundHeight(enemy.getTilePosition()) > BWAPI::Broodwar->getGroundHeight(BWAPI::TilePosition(unit.getEngagePosition())))
                        simRatio = simRatio * 2.0;

                    // Add their values to the simulation
                    enemyGrdSim += enemy.getVisibleGroundStrength() * simRatio;
                    enemyAirSim += enemy.getVisibleAirStrength() * simRatio;
                }
            };

            const auto simSelf = [&]() {
                for (auto &a : myUnits) {
                    auto &ally = a.second;
                    if (!canAddToSim(ally))
                        continue;

                    const auto widths = double(ally.getType().width() + ally.getTarget().getType().width()) / 2.0;
                    const auto distance = std::max(0.0, ally.getEngageDist() - widths);
                    const auto speed = 24.0 * ally.getSpeed();
                    auto simRatio = simulationTime - (distance / speed);

                    // If the unit doesn't affect this simulation
                    if (simRatio <= 0.0
                        || (ally.getSpeed() <= 0.0 && distance > 0.0)
                        || (ally.getPosition().getDistance(unit.getTarget().getPosition()) / speed) > simulationTime
                        || (ally.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode && unit.getTarget().getPosition().getDistance(ally.getPosition()) < 64.0))
                        continue;
                    
                    // High ground bonus
                    if (!ally.getType().isFlyer() && BWAPI::Broodwar->getGroundHeight(BWAPI::TilePosition(ally.getEngagePosition())) > BWAPI::Broodwar->getGroundHeight(TilePosition(ally.getTarget().getPosition())))
                        simRatio = simRatio * 2.0;                    

                    // Add their values to the simulation
                    myGrdSim += ally.getVisibleGroundStrength() * simRatio;
                    myAirSim += ally.getVisibleAirStrength() * simRatio;

                    // Check if air/ground sim needs to sync
                    if (!sync && simRatio > 0.0 && ((unit.getType().isFlyer() && !ally.getType().isFlyer()) || (!unit.getType().isFlyer() && ally.getType().isFlyer())))
                        sync = true;
                }
            };

            simEnemies();
            simSelf();

            newOutput.attackAirAsAir =          enemyAirSim > 0.0f ? myAirSim / enemyAirSim : 10.0f;
            newOutput.attackAirAsGround =       enemyGrdSim > 0.0f ? myAirSim / enemyGrdSim : 10.0f;
            newOutput.attackGroundAsAir =       enemyAirSim > 0.0f ? myGrdSim / enemyAirSim : 10.0f;
            newOutput.attackGroundasGround =    enemyGrdSim > 0.0f ? myGrdSim / enemyGrdSim : 10.0f;
            newOutput.shouldSynch =             sync;
        }
    }

    HorizonOutput getSimValue(BWAPI::Unit u, float simTime) {
        HorizonOutput newOutput;

        if (!u->exists() || u->getPlayer() != BWAPI::Broodwar->self())
            return newOutput;

        HorizonUnit &unit = myUnits[u];
        float unitToEngage = float(std::max(0.0, unit.getPosition().getDistance(unit.getEngagePosition()) / (24.0 * unit.getSpeed())));
        simulationTime = unitToEngage + simTime;
        simulate(newOutput, unit);
        return newOutput;
    }

    void updateUnit(BWAPI::Unit unit, BWAPI::Unit target) {

        if (!unit || !unit->exists())
            return;

        auto &u = unit->getPlayer() == BWAPI::Broodwar->self() ? myUnits[unit] : enemyUnits[unit];
        Maths::adjustSizes(unit);
        u.update(unit, target);

    }

    void removeUnit(BWAPI::Unit unit)
    {
        unit->getPlayer() == BWAPI::Broodwar->self() ? myUnits.erase(unit) : enemyUnits.erase(unit);
        Maths::adjustSizes(unit);
    }

    void HorizonUnit::update(BWAPI::Unit unit, BWAPI::Unit target) {
        auto t = unit->getType();
        auto p = unit->getPlayer();

        unitsTarget = nullptr;
        if (target) {
            if (myUnits.find(target) != myUnits.end())
                unitsTarget = &myUnits[target];
            else if (enemyUnits.find(target) != enemyUnits.end())
                unitsTarget = &enemyUnits[target];
        }

        thisUnit = unit;
        type = t;
        player = p;
        position = unit->getPosition();
        tilePosition = unit->getTilePosition();
        energy = unit->getEnergy();

        percentHealth = Maths::percentHealth(*this);
        groundRange = Maths::groundRange(*this);
        groundDamage = Maths::groundDamage(*this);
        airRange = Maths::airRange(*this);
        airDamage = Maths::airDamage(*this);
        speed = Maths::speed(*this);
        visGroundStrength = Maths::visGroundStrength(*this);
        visAirStrength = Maths::visAirStrength(*this);

        // Here is where you can add custom pathfinding to designate where you expect this Unit to engage its target and how far away the target is
        // Right now this assumes we are going to engage on a linear line to the target
        engageDist = position.getDistance(engagePosition);
        engagePosition = Maths::engagePosition(myUnits[unit]);
    }
}