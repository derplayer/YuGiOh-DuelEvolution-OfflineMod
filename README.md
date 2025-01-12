# Yu-Gi-Oh! Duel Evolution: Offline Mod
A mod that allows duel against offline AI by using the Duel Evolution's developer debug mode.
Inspired by [Yu-Gi-Oh! Master Duel offline mod](https://github.com/pixeltris/YgoMaster)

# Discord
https://discord.com/invite/GAKKaJYwF7
Help wanted!

# What is Yu-Gi-Oh! Duel Evolution?
Yu-Gi-Oh! Duel Evolution was an official free game released at december 2006.
You could get it from (the now shut-down) official website.
[More infos here.](https://yugioh.fandom.com/wiki/Yu-Gi-Oh!_Online:_Duel_Evolution)

# Usage
Only the 2006-12 client is supported. (yo2setup_061213_e.exe)
[Download](https://github.com/ysc3839/FontMod/releases) the package and just extract it into the installed game.

# Game closes after start
When the game closes instantly after start, your GPU has issues with Vulkan support. just remove following files:
`d3d8.dll`, `d3d9.dll`, `d3d10cre.dll`, `d3d11.dll`, `dxgi.dll`  
This will disable graphic fixes on modern operating system, but the game should then run.

# Antivirus issues
Due to injecting a debugger dll (TitanEngine.dll) it causes trouble with some AV software. One way to fix it would be to fix the softlock properly.
But i don't know how. Pull request are welcome!

## What has the mod to offer?:
* Fixing broken rendering on modern OS (via DXVK)
* Implementation of separate Player/NPC custom decks
* Fixed Deck Editor
	- Or at least as much as it is possible, in the working game version it was still at prototype stage.
	- To save the deck just press the "Exit" button. The deck will be saved to "deckOffline.ydc" file in the root folder of the game.
* Fix the UI dialogs for broken cards (resolves the softlock partially)
* Backporting of stuff from newer clients like
	- Card *.bin files
	- The new bin files were taken from the 2006-12-28 client, but are not thoroughly tested. But they do not introduce new cards to the pool, so it should be fine.

# FAQ:
* How to edit a NPC deck in-game?
	- Not possible at the moment. But you can edit the player deck, save it, make a copy and then rename the file from "deckOffline.ydc" to "deckOfflineCPU.ydc"

* What classifies as a broken card?
	- There are four types of broken card
	- Type A: The card is broken, but the mod fixes it.			Example: Kunai with Chain (The game would softlock without the mod)
	- Type B: The card works but has some card effect issues.	Example: Monster Reborn (The reborn monster will spawn face-down and not face-up)
	- Type C: The card works but has no effect coded.			Example: Cloak and Dagger (does nothing, has also no text in this version)
	- Type D: The card crashes the game.						Example: none found (yet?)
	- While Type A&B should be fixable in future, Type C&D would be too hard and are something that should be added to the game black list. 
	- Feel free to make an issue ticket for an broken card.

* The deck editor doesn't show the trunk cards anymore?
	-Press the button at the top right with the card trunk number. The filters are buggy.

* I made a cool deck, can it be included in this mod?
- Sure why not, submit it to this repo (the ydc file) and i will add it in the future as a community NPC deck. (Maybe even as an mini chibi NPC avatar walking on the game map with your nickname? haha)

* Why not using a newer client?
The debug duel mode is too broken inthose. I need help with this one, with someone that has more assembler/modding skills than me.

# Future TODOs:
* Stability fixes
* UI tweaks
* Blacklisting of broken cards
* NPC deck editor from newer version

# Far future TODOS:
* Walking around on the game MAPs to challenge NPCs
* Duel mode selection (normal duel/best of three/rock paper scissors)
* Card progression system (Power of Chaos style, one won battle = one new card in deck chest/trunk)
* Decks/NPCs from other Yu-Gi-Oh games
	- With level/difficulty scaling. (Power of Chaos style)
* Fixing of the broken card effects?
	- Similar to what ZAZA would do in his [Power of Chaos tutorial videos.](https://www.youtube.com/watch?v=7YEWebk3QCQ)
* Banlist
* Player avatar customisation
* Deck editor: load/save button functionality
* ...and more? ¯\_(ツ)_/¯

# Credits
derplayer (mod creator)
philyeahz (asm support)
hassanabuldahab (menu/font GFX)