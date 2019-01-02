# Horizon

Horizon is a free to use combat simulator for Broodwar AI. Horizon simulates combat using a short time horizon based on where an engagement will take place. Horizon returns a `struct` containing 4 separate `double` values it simulates and a `bool` which is a suggestion to synchronize your decisions with your air and ground units. 

### Adding Units
`Horizon::updateUnit(BWAPI::Unit, BWAPI::Unit)`

- Provide a `BWAPI::Unit` plus if this is your unit, you must provide an assigned enemy `BWAPI::Unit` as a target.
- Call this every frame on all visible units.

### Removing Units
`Horizon::removeUnit(BWAPI::Unit)`

- Provide a `BWAPI::Unit` to remove.

### Simulating Units 
`Horizon::getSimValue(BWAPI::Unit, double)`

- Provide a `BWAPI::Unit` to simulate and amount of time in seconds.
- Returns a `struct` containing all the simulated values.

### Modifying Horizon
There's a few customizations that you could add to Horizon to make it a bit better for your own usage.
- Pathfinding for ground units to find more accurate ground distance measurements to have more accurate results. 
- Value based outcomes to provide a more characteristically humanlike approach to combat, such as trading while ahead or sniping important units.
