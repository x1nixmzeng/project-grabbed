# Project Grabbed

A collection of reverse engineering efforts to explore and extract files from Grabbed by the Ghoulies.

The source files were originally written as part of a remake of the original Xbox game, and this repo is a subset of files from that app.


## Supported Files

Over the years I have researched other games developed by Rare, covering their titles on Xbox and Xbox360.

| Game                             | Platform   | Version             |  Formats                                             |
| -------------------------------- | ---------- | ------------------- | ---------------------------------------------------- |
| Grabbed by the Ghoulies          | Xbox       | n/a                 | localisation, audio, fonts, textures, (rough) models |
| Grabbed by the Ghoulies (demo)   | Xbox       | n/a                 | localisation, audio, fonts, textures, (rough) models |
| Kameo: Elements of Power (alpha) | Xbox       | `CAFF24.09.03.0026` | textures                                             |
| Conker: Live & Reloaded (demo)   | Xbox       | `CAFF24.09.03.0026` | textures                                             |
| Conker: Live & Reloaded          | Xbox       | `CAFF28.01.05.0031` | textures                                             |
| Perfect Dark Zero                | Xbox360    | `CAFF28.01.05.0031` | localisation                                         | 
| Viva Piñata                      | PC/Xbox360 | `CAFF07.08.06.0036` | localisation                                         |
| Banjo-Kazooie: Nuts & Bolts      | Xbox360    | `CAFF07.08.06.0036` | localisation                                         |


## Code Structure

The source is split into shared libraries (.lib) files that link with one of the tools. A rough structure is:

* `scripts` - directory for GENie project generation (https://github.com/bkaradzic/GENie)
* `src/external` - gitmodule dependencies live here
* `src/base` - provides shared file streaming, endian helper, zlib utilities, etc

The shared libraries for games are here too:

* `src/ghoulies` - main project for Grabbed by the Ghoulies
* `src/kameo` - main project for Kameo

(see the table above for the full list of supported games)

The main apps are stored in `src/tools`

* `src/tools/hashlookup` - reverse hash lookup for Grabbed by the Ghoulies, Kameo, and Pinata
* `src/tools/demandinfo` - loose bundle extractor for Grabbed by the Ghoulies

(there are various here)


## See Also

* Szkaradek123's model importer (Python, for Blender) - https://forum.xentax.com/viewtopic.php?f=16&t=12463
* RareWare Reddit - https://www.reddit.com/r/RareWare/
* Grabbed by the Ghoulies (Rare Wiki) - https://rarewiki.com/wiki/Grabbed_by_the_Ghoulies

