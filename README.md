# MCRS

MCRS is a free to use combat simulator for Broodwar AI. MCRS simulates combat using a short time horizon based on where an engagement will take place. MCRS returns a `struct` containing 4 separate `double` values it simulates and a `bool` which is a suggestion to synchronize your decisions with your air and ground units. 

### Adding Units
`MCRS::updateUnit(BWAPI::Unit, BWAPI::Unit)`

- Provide a `BWAPI::Unit` plus if this is your unit, you must provide an assigned enemy `BWAPI::Unit` as a target.
- Call this every frame on all visible units.

### Removing Units
`MCRS::removeUnit(BWAPI::Unit)`

- Provide a `BWAPI::Unit` to remove.

### Simulating Units 
`MCRS::getSimValue(BWAPI::Unit, double)`

- Provide a `BWAPI::Unit` to simulate and amount of time in seconds.
- Returns a `struct` containing all the simulated values.

### Modifying MCRS
There's a few customizations that you could add to MCRS to make it a bit better for your own usage.
- Pathfinding for ground units to find more accurate ground distance measurements to have more accurate results. 
- Value based outcomes to provide a more characteristically humanlike approach to combat, such as trading while ahead or sniping important units.
