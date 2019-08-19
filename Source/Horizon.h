#pragma once
#include <BWAPI.h>

namespace Horizon {

    struct HorizonOutput {
        float attackAirAsAir       = 0.0;
        float attackAirAsGround    = 0.0;
        float attackGroundAsAir    = 0.0;
        float attackGroundasGround = 0.0;
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
        float engageDist        = 0.0f;
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

        void setEngage(BWAPI::Position p)       { engagePosition = p; }

        BWAPI::Unit unit()                      { return thisUnit; }
        BWAPI::UnitType getType()               { return type; }
        BWAPI::Player getPlayer()               { return player; }
        BWAPI::Position getPosition()           { return position; }
        BWAPI::Position getEngagePosition()     { return engagePosition; }
        BWAPI::TilePosition getTilePosition()   { return tilePosition; }

        float getPercentHealth()               { return percentHealth; }
        float getVisibleGroundStrength()       { return visGroundStrength; }
        float getMaxGroundStrength()           { return maxGroundStrength; }
        float getVisibleAirStrength()          { return visAirStrength; }
        float getMaxAirStrength()              { return maxAirStrength; }
        float getGroundRange()                 { return groundRange; }
        float getAirRange()                    { return airRange; }
        float getGroundDamage()                { return groundDamage; }
        float getAirDamage()                   { return airDamage; }
        float getSpeed()                       { return speed; }
        float getEngageDist()                  { return engageDist; }
        int getEnergy()                        { return energy; }
    };

    /// Runs a simulation for this Unit and percent change of winning.    
    HorizonOutput getSimValue(BWAPI::Unit, float);

    /// Adds a Unit to Horizon.
    void updateUnit(BWAPI::Unit, BWAPI::Unit = nullptr);

    /// Removes the Unit from Horizon.
    void removeUnit(BWAPI::Unit);
};



