#pragma once

namespace Horizon::Maths {

    namespace {
        std::map<BWAPI::UnitSizeType, int> mySizes;
        std::map<BWAPI::UnitSizeType, int> enemySizes;
    }

    void adjustSizes(BWAPI::Unit unit) {
        int adj = unit->exists() ? 1 : -1;
        unit->getPlayer() == BWAPI::Broodwar->self() ? mySizes[unit->getType().size()] += adj : enemySizes[unit->getType().size()] += adj;
    }

    float survivability(HorizonUnit& unit) {
        const auto avgUnitSpeed = 4.34;
        const auto speed = log(unit.getSpeed() + avgUnitSpeed);
        const auto armor = 0.25 + float(unit.getType().armor() + unit.getPlayer()->getUpgradeLevel(unit.getType().armorUpgrade()));
        const auto health = log(float(unit.getType().maxHitPoints() + unit.getType().maxShields()));
        return speed * armor * health;
    }

    float splashModifier(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Archon || unit.getType() == BWAPI::UnitTypes::Terran_Firebat || unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 1.25f;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 4.00f;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode) return 2.50f;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Valkyrie || unit.getType() == BWAPI::UnitTypes::Zerg_Mutalisk) return 1.50f;
        if (unit.getType() == BWAPI::UnitTypes::Zerg_Lurker) return 2.00f;
        return 1.00f;
    }

    float effectiveness(HorizonUnit& unit) {
        auto effectiveness = 1.0f;
        auto sizes = unit.getPlayer() == BWAPI::Broodwar->self() ? enemySizes : mySizes;

        auto large = sizes[BWAPI::UnitSizeTypes::Large];
        auto medium = sizes[BWAPI::UnitSizeTypes::Medium];
        auto small = sizes[BWAPI::UnitSizeTypes::Small];
        auto total = float(large + medium + small);

        if (total > 0.0f) {
            if (unit.getType().groundWeapon().damageType() == BWAPI::DamageTypes::Explosive)
                effectiveness = ((large*1.00f) + (medium*0.75f) + (small*0.50f)) / total;
            else if (unit.getType().groundWeapon().damageType() == BWAPI::DamageTypes::Concussive)
                effectiveness = ((large*0.25f) + (medium*0.50f) + (small*1.0f)) / total;
        }
        return effectiveness;
    }

    float groundDamage(HorizonUnit& unit) {
        int upLevel = unit.getPlayer()->getUpgradeLevel(unit.getType().groundWeapon().upgradeType());
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Scarab_Damage)) return 125.0f;
            else return 100.0f;
        }
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 24.0f + (4.0f * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Terran_Firebat || unit.getType() == BWAPI::UnitTypes::Protoss_Zealot) return 16.0f + (2.0f * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 112.0f;
        return float(unit.getType().groundWeapon().damageAmount() + (unit.getType().groundWeapon().damageBonus() * upLevel));
    }

    float groundRange(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Dragoon && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge)) return 192.0f;
        if ((unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines))) return 160.0f;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 288.0f;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 256.0f;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) return 192.0f;
            return 160.0f;
        }
        return float(unit.getType().groundWeapon().maxRange());
    }

    float gWeaponCooldown(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 15.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 60.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 224.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Zergling && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands)) return 6.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->hasResearched(BWAPI::TechTypes::Stim_Packs)) return 7.5f;
        return float(unit.getType().groundWeapon().damageCooldown());
    }

    float groundDPS(HorizonUnit& unit) {
        const auto splash = splashModifier(unit);
        const auto damage = unit.getGroundDamage();
        const auto cooldown = gWeaponCooldown(unit);
        return damage > 1.0 ? splash * damage / cooldown : 0.0;
    }

    float visGroundStrength(HorizonUnit& unit) {
        if (unit.unit()->isMaelstrommed() || unit.unit()->isStasised())
            return 0.0f;
        return unit.getPercentHealth() * unit.getMaxGroundStrength();
    }

    float maxGroundStrength(HorizonUnit& unit) {
        // HACK: Some hardcoded values
        if (unit.getType() == BWAPI::UnitTypes::Terran_Medic)
            return 5.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Scarab || unit.getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine || unit.getType() == BWAPI::UnitTypes::Zerg_Egg || unit.getType() == BWAPI::UnitTypes::Zerg_Larva || unit.getGroundRange() <= 0.0f)
            return 0.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Interceptor)
            return 2.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Carrier) {
            float cnt = 0.0f;
            for (auto &i : unit.unit()->getInterceptors()) {
                if (i && !i->exists()) {
                    cnt += 2.0f;
                }
            }
            return cnt;
        }

        const auto dps = groundDPS(unit);
        const auto surv = log(survivability(unit));
        const auto eff = effectiveness(unit);
        const auto range = log(unit.getGroundRange());
        return dps * range * surv * eff;
    }

    float airDamage(HorizonUnit& unit) {
        int upLevel = unit.getPlayer()->getUpgradeLevel(unit.getType().airWeapon().upgradeType());
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker)	return 24.0f + (4.0f * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scout)	return 28.0f + (2.0f * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Terran_Valkyrie) return 48.0f + (8.0f * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 112.0f;
        return float(unit.getType().airWeapon().damageAmount() + (unit.getType().airWeapon().damageBonus() * upLevel));
    }

    float airRange(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Dragoon && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge)) return 192.0f;
        if ((unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines))) return 160.0f;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Goliath && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Charon_Boosters)) return 256.0f;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 288.0f;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) return 192.0f;
            return 160.0f;
        }
        return float(unit.getType().airWeapon().maxRange());
    }

    float aWeaponCooldown(HorizonUnit& unit)
    {
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 15.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 224.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Scourge) return 110.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Infested_Terran) return 500.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->hasResearched(BWAPI::TechTypes::Stim_Packs)) return 7.5f;
        return float(unit.getType().airWeapon().damageCooldown());
    }

    float airDPS(HorizonUnit& unit) {
        const auto splash = splashModifier(unit);
        const auto damage = unit.getAirDamage();
        const auto cooldown = aWeaponCooldown(unit);
        return  damage > 1.0 ? splash * damage / cooldown : 0.0;
    }

    float visAirStrength(HorizonUnit& unit) {
        if (unit.unit()->isMaelstrommed() || unit.unit()->isStasised())
            return 0.0f;
        return unit.getPercentHealth() * unit.getMaxAirStrength();
    }

    float maxAirStrength(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scarab || unit.getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine || unit.getType() == BWAPI::UnitTypes::Zerg_Egg || unit.getType() == BWAPI::UnitTypes::Zerg_Larva || unit.getAirRange() <= 0.0f)
            return 0.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Interceptor)
            return 2.0f;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Carrier) {
            float cnt = 0.0f;
            for (auto &i : unit.unit()->getInterceptors()) {
                if (i && !i->exists()) {
                    cnt += 2.0f;
                }
            }
            return cnt;
        }

        const auto dps = airDPS(unit);
        const auto surv = log(survivability(unit));
        const auto eff = effectiveness(unit);
        const auto range = log(unit.getAirRange());
        return dps * range * surv * eff;
    }

    float speed(HorizonUnit& unit) {
        float speed = float(unit.getType().topSpeed());

        if ((unit.getType() == BWAPI::UnitTypes::Zerg_Zergling && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Ultralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Anabolic_Synthesis)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Shuttle && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Gravitic_Drive)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Observer && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Gravitic_Boosters)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Zealot && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Leg_Enhancements)) || (unit.getType() == BWAPI::UnitTypes::Terran_Vulture && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Ion_Thrusters)))
            return speed * 1.5f;
        if (unit.getType() == BWAPI::UnitTypes::Zerg_Overlord && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Pneumatized_Carapace)) return speed * 4.01f;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scout && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)) return speed * 1.33f;
        if (unit.getType().isBuilding()) return 0.0f;
        return speed;
    }

    float percentHealth(HorizonUnit& unit) {
        return float(unit.unit()->getHitPoints() + (unit.unit()->getShields() / 2)) / float(unit.getType().maxHitPoints() + (unit.getType().maxShields() / 2));
    }

    BWAPI::Position engagePosition(HorizonUnit& unit) {
        if (!unit.hasTarget())
            return BWAPI::Positions::None;        

        int distance = (int)unit.getPosition().getDistance(unit.getTarget().getPosition());
        int range = unit.getTarget().getType().isFlyer() ? (int)unit.getAirRange() : (int)unit.getGroundRange();
        int leftover = distance - range;
        BWAPI::Position direction = (unit.getPosition() - unit.getTarget().getPosition()) * leftover / distance;

        if (distance > range)
           return (unit.getPosition() - direction);
        else
           return (unit.getPosition());
    }
}