# Project Grabbed

An old collection of reverse engineering efforts to explore and extract files from Grabbed by the Ghoulies.

The source files were originally written as part of a remake of the original game, and this repo is a subset of files from that app.

## Supported Files

Over the years I have researched a few other Rare games (mostly for localisation text or texturess):

* *Grabbed by the Ghoulies* `.bnl` bundle files
* *Grabbed by the Ghoulies* all other files `loctext`, `font`, `xwavebank`, `demand` loose bundled files
* *Kameo: Elements of Power* `.caf` textures
* *Kameo: Elements of Power* `.str` string tables (localisation)
* *Conker: Live & Reloaded* `default.rba` textures (with integrity check)
* *Viva Piñata* `.bnl` (PC and Xbox360, with integrity check)
* *Perfect Dark Zero* `.str` (Xbox360)
* *Banjo-Kazooie: Nuts & Bolts* `loctext files` (Xbox360)

## Code Structure

The source is split into shared libraries (.lib) files that link with one of the tools. A rough structure is:

* `src/external` - gitmodule dependencies live here
* `src/base` - provides shared file streaming, endian helper, zlib utilities, etc

The shared libraries for games are here too:

* `src/ghoulies` - main project for Grabbed by the Ghoulies
* `src/kameo` - main project for Kameo

(there are other games here)

The main .exe apps are stored in `src/tools`

* `src/tools/hashlookup` - reverse hash lookup for Grabbed by the Ghoulies, Kameo, and Pinata
* `src/tools/demandinfo` - loose bundle extractor for Grabbed by the Ghoulies

(there are several misc apps here)

## See Also

* Szkaradek123's model importer (Python, for Blender) - https://forum.xentax.com/viewtopic.php?f=16&t=12463
* RareWare Reddit - https://www.reddit.com/r/RareWare/

