# dscontrol

**dscontrol** is a utility that allows the Windows version of **Dark Souls: Remastered** to be controlled autonomously and deterministically. dscontrol plays back a predetermined series of gamepad inputs, as described in a [YAML script file](https://github.com/awforsythe/dscontrol/blob/main/data/script.yml), and records relevant gameplay state, such as player position, to a file.

Related projects:

- [**dscontrol-app**](https://github.com/awforsythe/dscontrol-app/) - UI for authoring scripts, triggering playback, and reviewing results. NYI.
- [**dscontrol-server**](https://github.com/awforsythe/dscontrol-server/) - Backend service that sits between that frontend and one or more instances of the dscontrol agent itself. Very NYI.

## Building from source

Visual C++ 2019 is required: you can download [Visual Studio 2019 Community](https://visualstudio.microsoft.com/vs/) and install the **Desktop Development with C++** workload.

- `git clone --recurse-submodules https://github.com/awforsythe/dscontrol-app.git`
- `cd dscontrol`
- `start build\dscontrol.sln`
- The path to `DarkSoulsRemastered.exe` is currently hardcoded; edit `DS1R_EXE_PATH` in main.cpp to change it
- Build the project

## Running a script

**IMPORTANT:** _**Make sure your game boots into Offline Mode before using this program.**_ If you have existing saves, I'd recommend backing them up, and not playing online again until you restore that backup.

You'll need to have the [**ViGEmBus driver**](https://github.com/ViGEm/ViGEmBus/releases) installed: this driver is required in order for the emulated gamepad functionality to work, and dscontrol will simply exit with an error message if ViGEmBus is not installed.

To test script playback:

- Edit `data\script.yml` to your liking
- Run `bin\dscontrol.exe` to launch the game
- Load up a character: dscontrol will wait until you're in-game
- You should automatically warp to a bonfire, teleport to the start position, and play back the inputs listed in the script file
- After the first run-through of the script, dscontrol will write out a file called `script.csv` that records the player position as a function of playback time
- dscontrol will continually loop, playing back the same script until you exit. The CSV file is only written once.
- Dark Souls 1 is finicky about gamepad input: if the character doesn't move, press Ctrl+Alt+Delete, then press Escape to return to the game. If that doesn't work, try unplugging any USB controllers and/or disabling USB Input Devices via Device Manager, then trying again from a fresh boot of the game.

## Third-Party Libraries

dscontrol uses:

- [**ViGEmClient**](https://github.com/ViGEm/ViGEmClient) for emulating an Xbox 360 gamepad - thanks to [nefarius](https://github.com/nefarius) and [megadrago88](https://github.com/megadrago88)
- [**rapidyaml**](https://github.com/biojppm/rapidyaml) for parsing YAML scripts - thanks to [biojppm](https://github.com/biojppm) and the other contributors to that project

## Acknowledgements

The Souls series has a very dedicated modding, speedrunning, and reverse-engineering community, and I never would have gotten this project off the ground if not for the work of that community. In particular:

The [**Dark Souls Remastered Cheat Engine Table**](https://fearlessrevolution.com/viewtopic.php?t=6856), by Phokz, was an invaluable resource while I was figuring out the basics of Cheat Engine. It clearly laid out a lot of the key data that's accessible in-memory, with the offsets and pointer chains required to find that data from a cold boot. Without that map to start from, I never would have been able to get a basic prototype working.

[**DSR-Gadget**](https://github.com/JKAnderson/DSR-Gadget), by [JKAnderson](https://github.com/JKAnderson), was instrumental in turning that prototype into a full-featured playback system. Not only did it present a clearer picture of the data structures used by the game, it also demonstrated a number of other features (postprocessing overrides, triggering bonfire warps via code injection) that I liberally stole from while writing my own C++ implementation.

The [**Souls Modding Wiki**](http://soulsmodding.wikidot.com/) and [**Souls Modding Discord**](https://discord.gg/mT2JJjx) have been a great help as well. Thanks to [Nordgaren](https://github.com/Nordgaren) for getting me pointed in the right direction at the start.

As I start working toward supporting other Souls games beyond DS1R, the work of the speedrunning community has become increasingly pivotal: those [LiveSplit AutoSplitters](https://github.com/LiveSplit/LiveSplit.AutoSplitters/blob/master/LiveSplit.AutoSplitters.xml) that automatically read in-game time are exactly the starting point I need. Those include:

- [**demonssouls.asl**](https://gist.github.com/CapitaineToinon/22aef17d50612c3e4e04cb7b5bdc434e) by [CapitaineToinon](https://github.com/CapitaineToinon)
- [**LiveSplit.DarkSoulsIGT**](https://github.com/CapitaineToinon/LiveSplit.DarkSoulsIGT) by [CapitaineToinon](https://github.com/CapitaineToinon)
- [**ASL-Scripts/DarkSoulsII.asl**](https://github.com/rythin-sr/ASL-Scripts/blob/master/DarkSoulsII.asl) by [rythin-sr](https://github.com/rythin-sr)
- [**Ds3Igt**](https://github.com/Jiiks/Ds3Igt) by [Jiiks](https://github.com/Jiiks)
- [**Sekiro.asl**](https://gist.github.com/pawREP/98c2828e7550caeb521d65c581f43a69) by [pawREP](https://github.com/pawREP)
