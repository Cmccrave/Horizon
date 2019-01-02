#pragma once

class HorizonUnit;

namespace Horizon::Maths {

    namespace {
        std::map<BWAPI::UnitSizeType, int> mySizes;
        std::map<BWAPI::UnitSizeType, int> enemySizes;
    }

    void adjustSizes(BWAPI::Unit unit) {
        int adj = unit->exists() ? 1 : -1;
        unit->getPlayer() == BWAPI::Broodwar->self() ? mySizes[unit->getType().size()] += adj : enemySizes[unit->getType().size()] += adj;
    }

    double survivability(HorizonUnit& unit) {
        double speed, armor, health;
        speed = (unit.getType().isBuilding()) ? 0.5 : std::max(1.0, log(unit.getSpeed()));
        armor = 2.0 + double(unit.getType().armor() + unit.getPlayer()->getUpgradeLevel(unit.getType().armorUpgrade()));
        health = log(((double)unit.getType().maxHitPoints() + (double)unit.getType().maxShields()) / 20.0);
        return speed * armor * health;
    }

    double splashModifier(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Archon || unit.getType() == BWAPI::UnitTypes::Terran_Firebat) return 1.25;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 1.25;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 6.00;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode) return 2.50;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Valkyrie || unit.getType() == BWAPI::UnitTypes::Zerg_Mutalisk) return 1.50;
        if (unit.getType() == BWAPI::UnitTypes::Zerg_Lurker) return 2.00;
        return 1.00;
    }

    double effectiveness(HorizonUnit& unit) {
        auto effectiveness = 1.0;
        auto sizes = unit.getPlayer() == BWAPI::Broodwar->self() ? enemySizes : mySizes;

        auto large = sizes[BWAPI::UnitSizeTypes::Large];
        auto medium = sizes[BWAPI::UnitSizeTypes::Medium];
        auto small = sizes[BWAPI::UnitSizeTypes::Small];
        auto total = double(large + medium + small);

        if (total > 0.0) {
            if (unit.getType().groundWeapon().damageType() == BWAPI::DamageTypes::Explosive)
                effectiveness = ((large*1.0) + (medium*0.75) + (small*0.5)) / total;
            else if (unit.getType().groundWeapon().damageType() == BWAPI::DamageTypes::Concussive)
                effectiveness = ((large*0.25) + (medium*0.5) + (small*1.0)) / total;
        }
        return effectiveness;
    }

    double groundDamage(HorizonUnit& unit) {
        int upLevel = unit.getPlayer()->getUpgradeLevel(unit.getType().groundWeapon().upgradeType());
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Scarab_Damage)) return 125.00;
            else return 100.00;
        }
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 24.0 + (4.0 * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Terran_Firebat || unit.getType() == BWAPI::UnitTypes::Protoss_Zealot) return 16.0 + (2.0 * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 112.0;
        return unit.getType().groundWeapon().damageAmount() + (unit.getType().groundWeapon().damageBonus() * upLevel);
    }

    double groundRange(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Dragoon && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge)) return 192.0;
        if ((unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines))) return 160.0;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 288.0;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 256.0;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) return 192.0;
            return 160.0;
        }
        return double(unit.getType().groundWeapon().maxRange());
    }

    double gWeaponCooldown(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 15.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Reaver) return 60.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 224.0;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Zergling && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands)) return 6.0;
        else if (unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->hasResearched(BWAPI::TechTypes::Stim_Packs)) return 7.5;
        return unit.getType().groundWeapon().damageCooldown();
    }

    double groundDPS(HorizonUnit& unit) {
        double splash, damage, range, cooldown;
        splash = splashModifier(unit);
        damage = unit.getGroundDamage();
        range = log(unit.getGroundRange());
        cooldown = gWeaponCooldown(unit);
        if (damage <= 0)
            return 0.0;
        return splash * damage * range / cooldown;
    }

    double visGroundStrength(HorizonUnit& unit) {
        if (unit.unit()->isMaelstrommed() || unit.unit()->isStasised())
            return 0.0;
        return unit.getPercentHealth() * unit.getMaxGroundStrength();
    }

    double maxGroundStrength(HorizonUnit& unit) {
        // HACK: Some hardcoded values
        if (unit.getType() == BWAPI::UnitTypes::Terran_Medic)
            return 5.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Scarab || unit.getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine || unit.getType() == BWAPI::UnitTypes::Zerg_Egg || unit.getType() == BWAPI::UnitTypes::Zerg_Larva || unit.getGroundRange() <= 0.0)
            return 0.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Interceptor)
            return 2.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Carrier) {
            double cnt = 0.0;
            for (auto &i : unit.unit()->getInterceptors()) {
                if (i && !i->exists()) {
                    cnt += 2.0;
                }
            }
            return cnt;
        }

        double dps, surv, eff;
        dps = groundDPS(unit);
        surv = log(survivability(unit));
        eff = effectiveness(unit);
        return dps * surv * eff;
    }

    double airDamage(HorizonUnit& unit) {
        int upLevel = unit.getPlayer()->getUpgradeLevel(unit.getType().airWeapon().upgradeType());
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker)	return 24.0 + (4.0 * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scout)	return 28.0 + (2.0 * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Terran_Valkyrie) return 48.0 + (8.0 * upLevel);
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 112.0;
        return unit.getType().airWeapon().damageAmount() + (unit.getType().airWeapon().damageBonus() * upLevel);
    }

    double airRange(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Dragoon && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge)) return 192.0;
        if ((unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines))) return 160.0;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Goliath && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Charon_Boosters)) return 256.0;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 288.0;
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) {
            if (unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells)) return 192.0;
            return 160.0;
        }
        return double(unit.getType().airWeapon().maxRange());
    }

    double aWeaponCooldown(HorizonUnit& unit)
    {
        if (unit.getType() == BWAPI::UnitTypes::Terran_Bunker) return 15.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_High_Templar) return 224.0;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Scourge) return 110.0;
        else if (unit.getType() == BWAPI::UnitTypes::Zerg_Infested_Terran) return 500.0;
        else if (unit.getType() == BWAPI::UnitTypes::Terran_Marine && unit.getPlayer()->hasResearched(BWAPI::TechTypes::Stim_Packs)) return 7.5;
        return unit.getType().airWeapon().damageCooldown();
    }

    double airDPS(HorizonUnit& unit) {
        double splash, damage, range, cooldown;
        splash = splashModifier(unit);
        damage = unit.getAirDamage();
        range = log(unit.getAirRange());
        cooldown = aWeaponCooldown(unit);
        if (damage <= 0)
            return 0.0;
        return splash * damage * range / cooldown;
    }

    double visAirStrength(HorizonUnit& unit) {
        if (unit.unit()->isMaelstrommed() || unit.unit()->isStasised())
            return 0.0;
        return unit.getPercentHealth() * unit.getMaxAirStrength();
    }

    double maxAirStrength(HorizonUnit& unit) {
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scarab || unit.getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine || unit.getType() == BWAPI::UnitTypes::Zerg_Egg || unit.getType() == BWAPI::UnitTypes::Zerg_Larva || unit.getAirRange() <= 0.0)
            return 0.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Interceptor)
            return 2.0;
        else if (unit.getType() == BWAPI::UnitTypes::Protoss_Carrier) {
            double cnt = 0.0;
            for (auto &i : unit.unit()->getInterceptors()) {
                if (i && !i->exists()) {
                    cnt += 2.0;
                }
            }
            return cnt;
        }

        double dps, surv, eff;
        dps = airDPS(unit);
        surv = log(survivability(unit));
        eff = effectiveness(unit);
        return dps * surv * eff;
    }

    double speed(HorizonUnit& unit) {
        double speed = unit.getType().topSpeed();

        if ((unit.getType() == BWAPI::UnitTypes::Zerg_Zergling && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Hydralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)) || (unit.getType() == BWAPI::UnitTypes::Zerg_Ultralisk && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Anabolic_Synthesis)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Shuttle && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Gravitic_Drive)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Observer && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Gravitic_Boosters)) || (unit.getType() == BWAPI::UnitTypes::Protoss_Zealot && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Leg_Enhancements)) || (unit.getType() == BWAPI::UnitTypes::Terran_Vulture && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Ion_Thrusters)))
            return speed * 1.5;
        if (unit.getType() == BWAPI::UnitTypes::Zerg_Overlord && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Pneumatized_Carapace)) return speed * 4.01;
        if (unit.getType() == BWAPI::UnitTypes::Protoss_Scout && unit.getPlayer()->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)) return speed * 1.33;
        if (unit.getType().isBuilding()) return 0.0;
        return speed;
    }

    double percentHealth(HorizonUnit& unit) {
        return double(unit.unit()->getHitPoints() + (unit.unit()->getShields() / 2)) / double(unit.getType().maxHitPoints() + (unit.getType().maxShields() / 2));
    }

    BWAPI::Position engagePosition(HorizonUnit& unit) {
        if (!unit.hasTarget())
            return BWAPI::Positions::None;

        double distance = unit.getPosition().getDistance(unit.getTarget().getPosition());
        double range = unit.getTarget().getType().isFlyer() ? unit.getAirRange() : unit.getGroundRange();
        double leftover = distance - range;
        BWAPI::Position direction = (unit.getPosition() - unit.getTarget().getPosition()) * leftover / distance;

        if (distance > range)
            return (unit.getPosition() - direction);
        return (unit.getPosition());
    }
}