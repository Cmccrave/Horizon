#include "MCRSim.h"
#include "MCRMaths.h"

MCRSimOutput MCRSim::getSimValue(BWAPI::Unit u, double simTime) {
	MCRUnit &unit = myUnits[u];
	MCRSimOutput newOutput;

	double unitToEngage = std::max(0.0, unit.getPosition().getDistance(unit.getEngagePosition()) / (24.0 * unit.getSpeed()));
	simulationTime = unitToEngage + simTime;
	bool sync = false;

	MCRInternalSimValue mySim = getMySim(unit, sync);
	MCRInternalSimValue enemySim = getEnemySim(unit);

	newOutput.attackAirAsAir = enemySim.air > 0.0 ? mySim.air / enemySim.air : DBL_MAX;
	newOutput.attackAirAsGround = enemySim.grd > 0.0 ? mySim.air / enemySim.grd : DBL_MAX;
	newOutput.attackGroundAsAir = enemySim.air > 0.0 ? mySim.grd / enemySim.air : DBL_MAX;
	newOutput.attackGroundasGround = enemySim.grd > 0.0 ? mySim.grd / enemySim.grd : DBL_MAX;

	return newOutput;
}

MCRInternalSimValue& MCRSim::getEnemySim(MCRUnit unit) {

	double enemyGrdSim;
	double enemyAirSim;
	double unitSpeed = unit.getSpeed() * 24.0;
	MCRInternalSimValue thisSim;

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

	thisSim.grd = enemyGrdSim;
	thisSim.air = enemyAirSim;
	return thisSim;
}

MCRInternalSimValue& MCRSim::getMySim(MCRUnit unit, bool& sync) {

	double myGrdSim;
	double myAirSim;
	MCRInternalSimValue thisSim;

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
		if (!sync && simRatio > 0.0 && ((unit.getType().isFlyer() && !ally.getType().isFlyer()) || (!unit.getType().isFlyer() && ally.getType().isFlyer())))
			sync = true;

		myGrdSim += ally.getVisibleGroundStrength() * simRatio;
		myAirSim += ally.getVisibleAirStrength() * simRatio;
	}

	thisSim.air = myAirSim;
	thisSim.grd = myGrdSim;
	return thisSim;
}

void MCRSim::myMCRUnit(BWAPI::Unit unit, BWAPI::Unit target) {
	myUnits[unit].update(unit);
	myUnits[unit].setTarget(&enemyUnits[target]);
	myUnits[unit].setEngagePosition(MCRMath::engagePosition(myUnits[unit]));
}

void MCRSim::enemyMCRUnit(BWAPI::Unit unit) {
	enemyUnits[unit].update(unit);
}

void MCRUnit::update(BWAPI::Unit unit, BWAPI::Unit target) {
	auto t = unit->getType();
	auto p = unit->getPlayer();

	unitsTarget = nullptr;
	thisUnit = unit;
	type = t;
	player = p;
	position = unit->getPosition();
	tilePosition = unit->getTilePosition();
	energy = unit->getEnergy();

	percentHealth = MCRMath::percentHealth(*this);
	groundRange = MCRMath::groundRange(*this);
	groundDamage = MCRMath::groundDamage(*this);
	airRange = MCRMath::airRange(*this);
	airDamage = MCRMath::airDamage(*this);
	speed = MCRMath::speed(*this);
	visGroundStrength = MCRMath::visGroundStrength(*this);
	visAirStrength = MCRMath::visAirStrength(*this);
}

MCRUnit::MCRUnit() {}

MCRSim* MCRSim::sim = nullptr;

MCRSim & MCRSim::Instance()
{
	if (!sim)
		sim = new MCRSim();
	return *sim;
}