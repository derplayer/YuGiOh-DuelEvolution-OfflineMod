Yu-Gi-Oh! Duel Evolution: Offline Mod - alpha test version 0.1

Thank you for joining this alpha test! Please DO NOT share this version online. People should enjoy it, when it is more stable and have a better experience.
This alpha test version has an expire date set to the end of the year. A public beta release will happen in the future, so you can update then.
Please report broken cards or other issues to me on Discord.

## What has the mod to offer?:
* Fixing broken rendering on modern OS (via DXVK)
	- If you have an older graphics card that does not support Vulkan API, please remove the all the dll files that start with "dx3*.dll and the dxgi.dll from this mod)
* Custom implementation of separate Player/NPC custom decks
* Fixed Deck Editor
	- Or at least as much as it is possible, in this version it was still at prototype stage.
	- To save the deck just press the "Exit" button. The deck will be saved to "deckOffline.ydc" file in the root folder of the game.
* Fix the UI dialogs for broken cards (resolves the softlock)
* Backporting of missing graphic assets
* Backporting card *.bin files
	- The new bin files were taken from the 2007-03 client, but are not thoroughly tested. But they do not introduce new cards to the pool, so it should be fine.

## FAQ:
Q: How to edit a NPC deck in this alpha?
A: Not possible at the moment. But you can edit the player deck, save it, make a copy and then rename the file from "deckOffline.ydc" to "deckOfflineCPU.ydc"

Q: What classifies as a broken card?
A: There are four types of broken card
	- Type A: The card is broken, but the mod fixes it.			Example: Kunai with Chain (The game would softlock without the mod)
	- Type B: The card works but has some card effect issues.	Example: Monster Reborn (The reborn monster will spawn face-down and not face-up)
	- Type C: The card works but has no effect coded.			Example: Cloak and Dagger (does nothing, has also no text in this version)
	- Type D: The card crashes the game.						Example: none found (yet?)
While Type A&B should be fixable in future, Type C&D would be too hard and are something that should be added to the game black list.

Q: The deck editor doesn't show the trunk cards anymore?
A: Press the button at the top right with the card trunk number. The filters are buggy.

Q: I made a cool deck, can it be included in this mod?
A: Sure why not, submit it to me (the ydc file) and i will add it in the future as a community NPC deck. (Maybe even as an mini chibi NPC avatar walking on the game map with your nickname? haha)

## Near future TODOs:
* Stability fixes
* UI tweaks
* Blacklisting of broken cards
* NPC deck editor

## Far future TODOS:
* Walking around on the game MAPs to challenge NPCs
* Duel mode selection (normal duel/best of three/rock paper scissors)
* Card progression system (Power of Chaos style, one won battle = one new card in deck chest/trunk)
	- Maybe also card packs opening
* Decks/NPCs from other Yu-Gi-Oh games
	- With level/difficulty scaling. (Power of Chaos style)
* Fixing of the broken card effects
	- Similar to what ZAZA would do in his Power of Chaos tutorial. (https://www.youtube.com/watch?v=7YEWebk3QCQ)
* Banlist
* Player avatar customisation
* Deck editor: load/save button functionality
* ...and more? ¯\_(ツ)_/¯

## Credits ##
derplayer (mod creator)
hassanabuldahab (menu GFX)

## Discord ##
https://discord.com/invite/GAKKaJYwF7