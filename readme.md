# Starfield Navigator

On 2022-06-12, Bethesda released the [first ingame footage](https://www.youtube.com/watch?v=zmb2FJGvnAw) of **Starfield**. Around 13:30 in the video, the starmap is visible for ~6 seconds. That scene was used to reconstruct the ingame map. The **Starfield Navigator** is a small program uses that data in a couple of ways. This site contains the source of the program, a downloadable version as well as discussion of the method, current efforts and findings.

https://user-images.githubusercontent.com/6044318/177479550-c8bf36b0-9571-4ade-a45a-d1f4a05bc733.mp4

<br/><br/>

:arrow_down: :floppy_disk: [**Download current version**](https://github.com/s9w/starfield-navigator/releases/latest/download/starfield_navigator.zip) :floppy_disk: :arrow_down:

:parrot: :rabbit2: :turtle: [Changelog is here](changelog.md) :sauropod: :frog: :elephant:

(If you get a dll error, you're missing a recent [C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe))

<br/><br/>

## Current effort
100% of the visible stars were identified. For some, identification is tricky because it's unknown how exactly they deal with multiple-star systems and the time difference (for example Xi Scorpii).

Next up will be tools to help reconstruct the non-visible stars.

## Insights
A couple of insights were gained from the data:
- Of the "over 100" stars announced for the final game, 75 were visible. Seven were labeled, three of them match real stars in position, name and other properties
- The game uses [Gliese numbers](https://en.wikipedia.org/wiki/Gliese_Catalogue_of_Nearby_Stars) and the known systems had correct relations to each other. Therefore it's likely that all stars are real, some of them renamed
- Over ten other unlabeled systems have been identified
- The systems with unknown labels (NARION, VOLII, CHEYENNE, JAFFA) are renames of real systems
- Of the 7 named systems in the trailer, the 5 leftmost ones had their name on the right side of the planet. The rightmost ones had their name on the left side. Barring other explanations, this might imply that the horizontal center of the map is somewhere between JAFFA and VOLII

The visible map is mostly to the right of SOL. In [this video](https://www.youtube.com/watch?v=xaNwtw7bhyk), it says "Our game is set [...] in an area that extends outward from our solar system for approximately 50 light years". That weakly implies that sol is somewhat centered. This collides with the observed fact that we already saw stars pretty far out of a 50 LY sphere around sol. But while the video is official, it's pretty questionable overall: The video animation doesn't match what the spoken description. Also the settled systems are called an "area of **our** solar system", which almost certainly is not correct.
