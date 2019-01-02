#include "Horizon.h"
#include "Maths.h"

namespace Horizon {

    namespace {
        double simulationTime;
        std::map<BWAPI::Unit, HorizonUnit> enemyUnits;
        std::map<BWAPI::Unit, HorizonUnit> myUnits;
        
        void simulate(HorizonOutput& newOutput, HorizonUnit& unit) {

            double enemyGrdSim;
            double enemyAirSim;
            double myGrdSim;
            double myAirSim;
            double unitSpeed = unit.getSpeed() * 24.0;

            // Check every enemy unit being in range of the target
            for (auto &e : enemyUnits) {
                auto &enemy = e.second;

                // Ignore workers and useless units
                if (!enemy.unit() || enemy.getType().isWorker() || (enemy.unit() && enemy.unit()->exists() && (enemy.unit()->isStasised() || enemy.unit()->isMorphing())))
                    continue;

                // Distance parameters
                double widths = (double)enemy.getType().tileWidth() * 16.0 + (double)unit.getType().tileWidth() * 16.0;
                double enemyRange = (unit.getType().isFlyer() ? enemy.getAirRange() : enemy.getGroundRange());
                double airDist = enemy.getPosition().getDistance(unit.getPosition());

                // True distance
                double distance = airDist - enemyRange - widths;

                // Sim values
                double enemyToEngage = 0.0;
                double simRatio = 0.0;
                double speed = enemy.getSpeed() * 24.0;

                // If enemy can move, distance/speed is time to engage
                if (speed > 0.0) {
                    enemyToEngage = std::max(0.0, distance / speed);
                    simRatio = std::max(0.0, simulationTime - enemyToEngage);
                }

                // If enemy can't move, it must be in range of our engage position to be added
                else if (enemy.getPosition().getDistance(unit.getEngagePosition()) - enemyRange - widths <= 0.0) {
                    enemyToEngage = std::max(0.0, distance / unitSpeed);
                    simRatio = std::max(0.0, simulationTime - enemyToEngage);
                }
                else
                    continue;

                // High ground check
                if (!enemy.getType().isFlyer() && BWAPI::Broodwar->getGroundHeight(enemy.getTilePosition()) > BWAPI::Broodwar->getGroundHeight(BWAPI::TilePosition(unit.getEngagePosition())))
                    simRatio = simRatio * 2.0;

                enemyGrdSim += enemy.getVisibleGroundStrength() * simRatio;
                enemyAirSim += enemy.getVisibleAirStrength() * simRatio;
            }

            // Check every ally being in range of the target
            for (auto &a : myUnits) {
                auto &ally = a.second;

                if (!ally.hasTarget() || !ally.unit() || ally.unit()->isStasised() || ally.unit()->isMorphing())
                    continue;

                // Setup distance values
                double dist = ally.getPosition().getDistance(ally.getEngagePosition());
                double widths = (double)ally.getType().tileWidth() * 16.0 + (double)unit.getType().tileWidth() * 16.0;
                double speed = 24.0 * ally.getSpeed();

                // Setup true distance
                double distance = dist - widths;

                if (ally.getPosition().getDistance(unit.getEngagePosition()) / speed > simulationTime)
                    continue;

                double allyToEngage = std::max(0.0, (distance / speed));
                double simRatio = std::max(0.0, simulationTime - allyToEngage);

                // High ground check
                if (!ally.getType().isFlyer() && BWAPI::Broodwar->getGroundHeight(BWAPI::TilePosition(ally.getEngagePosition())) > BWAPI::Broodwar->getGroundHeight(BWAPI::TilePosition(ally.getTarget().getPosition())))
                    simRatio = simRatio * 2.0;

                // Synchronize check
                if (!newOutput.shouldSynch && simRatio > 0.0 && ((unit.getType().isFlyer() && !ally.getType().isFlyer()) || (!unit.getType().isFlyer() && ally.getType().isFlyer())))
                    newOutput.shouldSynch = true;

                myGrdSim += ally.getVisibleGroundStrength() * simRatio;
                myAirSim += ally.getVisibleAirStrength() * simRatio;
            }

            newOutput.attackAirAsAir = enemyAirSim > 0.0 ? myAirSim / enemyAirSim : DBL_MAX;
            newOutput.attackAirAsGround = enemyGrdSim > 0.0 ? myAirSim / enemyGrdSim : DBL_MAX;
            newOutput.attackGroundAsAir = enemyAirSim > 0.0 ? myGrdSim / enemyAirSim : DBL_MAX;
            newOutput.attackGroundasGround = enemyGrdSim > 0.0 ? myGrdSim / enemyGrdSim : DBL_MAX;
        }
    }

    HorizonOutput getSimValue(BWAPI::Unit u, double simTime) {
        HorizonOutput newOutput;

        if (!u->exists() || u->getPlayer() != BWAPI::Broodwar->self())
            return newOutput;

        HorizonUnit &unit = myUnits[u];
        double unitToEngage = std::max(0.0, unit.getPosition().getDistance(unit.getEngagePosition()) / (24.0 * unit.getSpeed()));
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
        u.setTarget(&enemyUnits[target]);
        u.setEngage(Maths::engagePosition(myUnits[unit]));
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
    }
}