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
                    || (!u.unit()->isCompleted() && u.unit()->exists())
                    || (u.unit()->exists() && (u.unit()->isStasised() || u.unit()->isMorphing()))
                    || (u.getVisibleAirStrength() <= 0.0 && u.getVisibleGroundStrength() <= 0.0))
                    return false;
                return true;
            };

            const auto simEnemies = [&]() {
                for (auto &e : enemyUnits) {
                    HorizonUnit &enemy = e.second;
                    if (!canAddToSim(enemy))
                        continue;

                    const auto deadzone = (enemy.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode && unit.getTarget().getPosition().getDistance(enemy.getPosition()) < 64.0) ? 64.0 : 0.0;
                    const auto widths = double(enemy.getType().width() + unit.getType().width()) / 2.0;
                    const auto enemyRange = (unit.getType().isFlyer() ? enemy.getAirRange() : enemy.getGroundRange());

                    // If enemy is stationary, it must be in range of the engage position
                    if (enemy.getSpeed() <= 0.0) {
                        auto engageDistance = enemy.getPosition().getDistance(unit.getEngagePosition()) - enemyRange - widths;
                        if (engageDistance > 64.0)
                            continue;
                    }

                    const auto distance = std::max(0.0, enemy.getPosition().getDistance(unit.getPosition()) - enemyRange - widths + deadzone);
                    const auto speed = enemy.getSpeed() > 0.0 ? 24.0 * enemy.getSpeed() : 24.0 * unit.getSpeed();
                    const auto simRatio =  simulationTime - (distance / speed);

                    if (simRatio <= 0.0)
                        continue;

                    // Add their values to the simulation
                    enemyGrdSim += enemy.getVisibleGroundStrength() * simRatio;
                    enemyAirSim += enemy.getVisibleAirStrength() * simRatio;
                }
            };

            const auto simSelf = [&]() {
                for (auto &a : myUnits) {
                    HorizonUnit &ally = a.second;
                    if (!canAddToSim(ally))
                        continue;

                    const auto deadzone = (ally.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode && unit.getTarget().getPosition().getDistance(ally.getPosition()) < 64.0) ? 64.0 : 0.0;
                    const auto engDist = ally.getEngageDist();
                    const auto widths = double(ally.getType().width() + ally.getTarget().getType().width()) / 2.0;
                    const auto allyRange = (unit.getTarget().getType().isFlyer() ? ally.getAirRange() : ally.getGroundRange());
                    const auto speed = 24.0 * ally.getSpeed();
                    const auto distance = std::max(0.0, engDist - widths + deadzone);
                    const auto simRatio = simulationTime - (distance / speed);

                    // If the unit doesn't affect this simulation
                    if (simRatio <= 0.0 || (ally.getSpeed() <= 0.0 && ally.getPosition().getDistance(unit.getTarget().getPosition()) - allyRange - widths > 64.0))
                        continue;

                    if (!sync && simRatio > 0.0 && ((unit.getType().isFlyer() && !ally.getType().isFlyer()) || (!unit.getType().isFlyer() && ally.getType().isFlyer())))
                        sync = true;

                    // Add their values to the simulation
                    myGrdSim += ally.getVisibleGroundStrength() * simRatio;
                    myAirSim += ally.getVisibleAirStrength() * simRatio;
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