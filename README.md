# Left4TAS
TAS Plugin for Left 4 Dead 2

The current version is considered the basis for further promotion of the toolkit

At the moment there's the most basic things for creating Tool-Assisted Speedrun in Left 4 Dead 2 running on the Source Engine: frame-by-frame input, autostrafer; also callbacks (execution of .cfg files and call of VScript function `Left4TAS_Callbacks`), various console commands/variables and HUD

~~Currently you can use the plugin for making solo segments. Next I want to add splitscreen support (duo) and later support of co-op mode~~

Some features were based on [SourcePauseTool](https://github.com/YaLTeR/SourcePauseTool "SourcePauseTool")

# Wiki
How to load the plugin, some examples, description of modules (cvars/commands) and other stuff [here](https://github.com/r47t/Left4TAS/wiki "here")

# Some notes
If you use the plugin, **DO NOT** use VScripts/SourceMod to manipulate the game state (spawn/change items, CI, SI, bosses, use random generator etc), it means don't create cheated runs. The only purpose is for debugging, creating helper functions and other stuff which will not modify the game (there may be exceptions due to the technical capabilities of the plugin but I'm thinking about it)

# Project
The project is written in Visual Studio 2019 and doesn't support Linux platform

You must have unofficial [L4D2 SDK](https://github.com/alliedmodders/hl2sdk/tree/l4d2 "L4D2 SDK") from AlliedModders and the disassembler library [libdasm](https://github.com/jtpereyda/libdasm "libdasm") in the project

# Post-TLS versions
I'm not going to support the versions that came out during the TLS update and after

# Issues
If you are playing on version 2.1.5.5 or below and you found bugs or the plugin was unable to initialize all of its functions, please report it. Write the version of the game and a message in console that the plugin wrote during the failed initialization but make sure you disabled all 3rd party plugins (SourceMod, MetaMod: Source etc; actually it's better to don't use them). If the problem is different, then I will try to solve it if possible
