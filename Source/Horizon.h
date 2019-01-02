#pragma once
#include <BWAPI.h>

namespace Horizon {

    struct HorizonOutput {
        double attackAirAsAir       = 0.0;
        double attackAirAsGround    = 0.0;
        double attackGroundAsAir    = 0.0;
        double attackGroundasGround = 0.0;
        bool shouldSynch            = false;
    };

    class HorizonUnit {

        HorizonUnit* unitsTarget;

        float percentHealth     = 0.0f;
        float groundRange       = 0.0f;
        float airRange          = 0.0f;
        float groundDamage      = 0.0f;
        float airDamage         = 0.0f;
        float speed             = 0.0f;
        float visGroundStrength = 0.0f; 
        float visAirStrength    = 0.0f;
        float maxGroundStrength = 0.0f;
        float maxAirStrength    = 0.0f;
        int shields             = 0;
        int health              = 0;
        int energy              = 0;

        BWAPI::Unit thisUnit             = nullptr;
        BWAPI::UnitType type             = BWAPI::UnitTypes::None;
        BWAPI::Player player             = nullptr;
        BWAPI::Position position         = BWAPI::Positions::None;
        BWAPI::Position engagePosition   = BWAPI::Positions::None;
        BWAPI::TilePosition tilePosition = BWAPI::TilePositions::None;
    public:
        HorizonUnit() { };
        void update(BWAPI::Unit unit, BWAPI::Unit target = nullptr);

        bool hasTarget()                        { return unitsTarget != nullptr; }
        HorizonUnit &getTarget()                { return *unitsTarget; }

        void setTarget(HorizonUnit* t)          { unitsTarget = t; }
        void setEngage(BWAPI::Position p)       { engagePosition = p; }

        BWAPI::Unit unit()                      { return thisUnit; }
        BWAPI::UnitType getType()               { return type; }
        BWAPI::Player getPlayer()               { return player; }
        BWAPI::Position getPosition()           { return position; }
        BWAPI::Position getEngagePosition()     { return engagePosition; }
        BWAPI::TilePosition getTilePosition()   { return tilePosition; }

        double getPercentHealth()               { return percentHealth; }
        double getVisibleGroundStrength()       { return visGroundStrength; }
        double getMaxGroundStrength()           { return maxGroundStrength; }
        double getVisibleAirStrength()          { return visAirStrength; }
        double getMaxAirStrength()              { return maxAirStrength; }
        double getGroundRange()                 { return groundRange; }
        double getAirRange()                    { return airRange; }
        double getGroundDamage()                { return groundDamage; }	
        double getAirDamage()                   { return airDamage; }
        double getSpeed()                       { return speed; }
        int getEnergy()                         { return energy; }
    };

    /// Runs a simulation for this Unit and percent change of winning.    
    HorizonOutput getSimValue(BWAPI::Unit, double);

    /// Adds a Unit to Horizon.
    void updateUnit(BWAPI::Unit, BWAPI::Unit = nullptr);

    /// Removes the Unit from Horizon.
    void removeUnit(BWAPI::Unit);
};



