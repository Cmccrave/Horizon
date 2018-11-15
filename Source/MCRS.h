#pragma once
#include <BWAPI.h>

struct MCRSOutput {
	double attackAirAsAir = 0.0;
	double attackAirAsGround = 0.0;
	double attackGroundAsAir = 0.0;
	double attackGroundasGround = 0.0;
	bool shouldSynch = false;
};

class MCRSUnit {

	MCRSUnit* unitsTarget;

	double percentHealth, groundRange, airRange, groundDamage, airDamage, speed;						// StarCraft stats
	double visGroundStrength, visAirStrength, maxGroundStrength, maxAirStrength;				// McRave stats
	int shields, health, energy;

	BWAPI::Unit thisUnit;
	BWAPI::UnitType type;
	BWAPI::Player player;
	BWAPI::Position position, engagePosition;
	BWAPI::TilePosition tilePosition;
public:
	MCRSUnit();
	void update(BWAPI::Unit unit, BWAPI::Unit target = nullptr);
	bool hasTarget()						{ return unitsTarget != nullptr; }
	MCRSUnit &getTarget()					{ return *unitsTarget; }

	void setTarget(MCRSUnit* target)			{ unitsTarget = target; }

	void setEngagePosition(BWAPI::Position position) {
		engagePosition = position; }
	
	BWAPI::Unit unit()						{ return thisUnit; }
	BWAPI::UnitType getType()				{ return type; }
	BWAPI::Player getPlayer()				{ return player; }
	BWAPI::Position getPosition()			{ return position; }
	BWAPI::Position getEngagePosition()		{ return engagePosition; }
	BWAPI::TilePosition getTilePosition()	{ return tilePosition; }	

	// Starcraft Stats
	double getPercentHealth()				{ return percentHealth; }				// Returns the units health and shield percentage		
	double getVisibleGroundStrength()		{ return visGroundStrength; }			// Returns the units visible ground strength		
	double getMaxGroundStrength()			{ return maxGroundStrength; }			// Returns the units max ground strength		
	double getVisibleAirStrength()			{ return visAirStrength; }				// Returns the units visible air strength		
	double getMaxAirStrength()				{ return maxAirStrength; }				// Returns the units max air strength
	double getGroundRange()					{ return groundRange; }					// Returns the units ground range including upgrades		
	double getAirRange()					{ return airRange; }					// Returns the units air range including upgrades				
	double getGroundDamage()				{ return groundDamage; }				// Returns the units ground damage (including most upgrades)		
	double getAirDamage()					{ return airDamage; }					// Returns the units air damage (including most upgrades)		
	double getSpeed()						{ return speed; }						// Returns the units movement speed in pixels per frame including upgrades
	int getEnergy()							{ return energy; }
};

class MCRS {

private:
	// Variables for calculating local strengths
	double simulationTime;
	static MCRS* sim;

	std::map<BWAPI::Unit, MCRSUnit> enemyUnits;
	std::map<BWAPI::Unit, MCRSUnit> myUnits;
	void simulate(MCRSOutput&, MCRSUnit&);

public:
		
	// Returns a simulation for ground and air and a boolean of whether it is suggested you synchronize the simulations.
	MCRSOutput getSimValue(BWAPI::Unit, double);

	// Call this every frame on every unit you have, you must supply a target for the sim if it's your own unit.
	void updateUnit(BWAPI::Unit, BWAPI::Unit = nullptr);

	// Remove any units as they die
	void removeUnit(BWAPI::Unit);

	static MCRS &Instance();
};



