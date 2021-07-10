# Left4TAS
TAS Plugin for Left 4 Dead 2

The current version is considered the basis for further promotion of the toolkit

At the moment there is the most basic things for creating a TAS on the Source Engine: frame-by-frame input, autostrafer; also callbacks (execution of .cfg files and VScripts), various console commands/variables and HUD

Some features were based on [SourcePauseTool](https://github.com/YaLTeR/SourcePauseTool "SourcePauseTool")

# Wiki
How to load the plugin, description of modules (cvars and commands) and other things: [here](https://github.com/r47t/Left4TAS/wiki "here")

# Some notes
If you use the plugin, please don't use VScripts/SourceMod to manipulate the game state (spawn items, CI, SI, bosses, use random generator etc)

# Project
The project is written in Visual Studio 2019

You must have an unofficial [L4D2 SDK](https://github.com/alliedmodders/hl2sdk/tree/l4d2 "L4D2 SDK") from AlliedModders and the disassembler library [libdasm](https://github.com/jtpereyda/libdasm "libdasm") in the project

# Post-TLS versions
I'm not going to support the versions that came out during the TLS update and after

# Issues
If you are playing on version 2.1.5.5 and below, and encountered bugs or if the plugin was unable to initialize all of its functions, please report it. Write the version of the game and a message in console that the plugin wrote during the failed initialization but make sure you disabled all 3rd party plugins (SourceMod, MetaMod: Source). If the problem is different, then I will try to solve it if possible
