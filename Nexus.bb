[color=#6aa84f][size=5][b]Description[/b][/size][/color]

QuickLoot IE allows you to look through and take items from containers without the need to open the inventory screen every time. Compared to its predecessor [url=https://www.nexusmods.com/skyrimspecialedition/mods/69980]QuickLoot EE[/url], this version comes with a whole host of new features, improved compatibility with other mods, and expanded configuration options.

[color=#6aa84f][size=5][b]Version Compatibility[/b][/size][/color]

QuickLoot IE supports all game versions in the range of [b]1.5.97[/b] to [b]1.6.1179[/b], but if you are on a version older than [b]1.6.1130[/b], you need to have [url=https://www.nexusmods.com/skyrimspecialedition/mods/106441]BEES[/url] installed.
Support for Skyrim VR is in the works. In the meantime you can use [url=https://www.nexusmods.com/skyrimspecialedition/mods/102094]this VR version[/url] of the original QuickLoot EE.

You cannot use reskins made for QuickLoot EE with our version of the mod since the menu was completely rewritten from scratch. To indicate this we've decided to rename the swf file to [b]LootMenuIE.swf[/b] to avoid confusion. Should [b]LootMenu.swf[/b] still be present due to an incompatible UI patch, then a warning message will show every time you start the game. To prevent this, either hide the file in your mod manager or disable the check in [b]QuickLootIE.json[/b].

[color=#6aa84f][size=5][b]Features[/b][/size][/color]

[list]
[*][color=#f6b26b][b]Basic functionality[/b][/color]
Looking at any container or corpse displays a popup window, allowing you to inspect its inventory without opening the regular container menu or pausing the game. From there you can select and take out stacks of items or open the full inventory screen.

The new and improved loot menu now features a value/weight column and little indicator arrows to signal whether you can scroll up or down. It also dynamically resizes to only take up as much space as needed to fit the displayed items and preserves the last selected item when you briefly look away.
[/list]
[list]
[*][color=#f6b26b][b]Mod integrations and compatibility[/b][/color]
QuickLoot IE introduces native compatibility with several mods that previously didn't work well together or required patches:

[list]
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/85702]Inventory Interface Information Injector[/url] - Support for custom colored icons.
Tested with the following
[list]
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/68557]Aura's Inventory Tweaks[/url]
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/116713]Kome's Inventory Tweaks[/url]
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/106432]I4 Weapon Icons Overhaul[/url][/list]
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/46358]Completionist[/url] - Full support for item color, text prefixes and icons.
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/26790]No Lockpick Activate[/url] - No longer needed as this is not a setting.
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/38529]The Curator's Companion[/url] - Icons for needed, found and displayed items.
[*] [url=https://mod.pub/skyrim-se/36-artifact-tracker]Artifact Tracker[/url] - Acts as a drop-in replacement for TCC.
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/52743]Oblivion Interaction Icons[/url] - Icons now show up in the loot menu button bar.
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/27859]Skyrim Souls RE[/url] - Loot menu no longer shows on top of other menus.
[*] [url=https://www.nexusmods.com/skyrimspecialedition/mods/56767]Simple Activate SKSE[/url] - Fixed flickering vanilla activation prompt.
[/list][/list]
[list]
[*][color=#f6b26b][b]MCM settings[/b][/color]
Many aspects of QuickLoot IE's behavior can be configured via the included MCM:

[list]
[*] Fine grained control over when the loot menu should show up
[*] Position and size of the loot menu
[*] The number of items to show without the need to scroll
[*] Individual toggles for the various optional icons
[*] Which info columns to show and in what order
[*] Key bindings for all QuickLoot actions (+ save/load presets)
[/list][/list]
[color=#6aa84f][size=5][b]Frequently Asked Questions[/b][/size][/color]

[list]
[*][color=#f6b26b][b]I get a popup talking about "LootMenu.swf"[/b][/color]
This means that you installed a loot menu reskin created for an older version of QuickLoot, which is incompatible with QuickLoot IE.
Check which mod provides [b]Interface/LootMenu.swf[/b] and remove the file.
If for some reason you are unable to do so, you can disable the check in [b]QuickLootIE.json[/b].
[/list]
[list]
[*][color=#f6b26b][b]I get a popup talking about the MCM quest[/b][/color]
Make sure QuickLootIE.esp exists and is enabled in your load order.
If it is and you still get the popup, you're probably on a version before [b]1.6.1130[/b] and did not install [url=https://www.nexusmods.com/skyrimspecialedition/mods/106441]BEES[/url].
[/list]
[list]
[*][color=#f6b26b][b]The loot menu doesn't show up when I look at a container[/b][/color]
This is most likely caused by another mod adding an overlay menu that isn't properly flagged as such.
Check your QuickLoot IE log ([b]Documents/My Games/Skyrim Special Edition/SKSE/QuickLootIE.log[/b]) for a line saying "LootMenu disabled because a blocking menu is open."
At the end of the line it will state the name of the menu. Add this to the menu whitelist in [b]QuickLootIE.json[/b].
[/list]
[list]
[*][color=#f6b26b][b]I have another issue[/b][/color]
Bugs and feature requests are best posted directly to the [url=https://github.com/MissCorruption/QuickLootIE/issues]GitHub repository[/url], but we also try to keep an eye on the Nexus page. Before posting, please read the sticky posts in the comments section and use the search function to check whether someone else has already informed us of the issue.
[/list]
[color=#6aa84f][size=5][b]Technical Documentation[/b][/size][/color]

There are two config files that can be used to modify the behavior of QuickLoot IE.

[list]
[*][b]SKSE/Plugins/QuickLootIE/DefaultConfig.json[/b]
This file contains the same settings as the in-game configuration menu. These settings are loaded automatically when starting a fresh save.
You can also manually load and overwrite this file using the [b]Load Profile[/b] and [b]Save Profile[/b] buttons in the MCM.
Although the file does not exist by default, it can be provided by mod list authors to pre-configure the mod.
For this functionality to be available, [url=https://www.nexusmods.com/skyrimspecialedition/mods/13048]PapyrusUtil[/url] must be installed.
[/list]
[list]
[*][b]SKSE/Plugins/QuickLootIE.json[/b]
This file contains some more niche technical settings that most players won't have to interact with.
It allows you to control some debug features, whitelist improperly flagged menus and exclude containers from being searchable by QuickLoot IE.
[/list]
Beyond the json blacklist, containers can also be excluded from use with QuickLoot IE by assigning the keyword [b]QuickLootIE_Exclude[/b] (QuickLootIE.esp|0x002) to them. Should this not be flexible enough for your needs, QuickLoot IE provides an [url=https://github.com/MissCorruption/QuickLootIE/blob/master/include/QuickLootAPI.h]SKSE API[/url], which you can use in your own SKSE plugin.

[color=#6aa84f][size=5][b]Credits[/b][/size][/color]

QuickLoot IE is a fork of [url=https://www.nexusmods.com/skyrimspecialedition/mods/69980]QuickLoot EE[/url] by Eloquence, which in turn is a fork of [url=https://www.nexusmods.com/skyrimspecialedition/mods/21085]QuickLoot RE[/url] by Ryan McKenzie.

<credits banner goes here>

[b][size=3]Support the Author[/size][/b]
Support me on [url=https://ko-fi.com/misscorruption]Ko-Fi[/url] or check out the [url=https://github.com/MissCorruption/QuickLootIE]GitHub repository[/url] containing the source code!