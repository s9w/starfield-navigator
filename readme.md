# Starfield Navigator

On 2022-06-12, Bethesda released the [first ingame footage](https://www.youtube.com/watch?v=zmb2FJGvnAw) of **Starfield**. Around 13:30 in the video, the starmap is visible for ~6 seconds. That scene was used to track all visible stars and reconstruct their ingame position. The footage included seven named stars, three of them are real star systems (Sol, Alpha Centauri and Porrima). The real-life position of those stars were used to calibrate the reconstruction.

The Starfield Navigator is a small program uses that data in a couple of ways:

https://user-images.githubusercontent.com/6044318/177479550-c8bf36b0-9571-4ade-a45a-d1f4a05bc733.mp4

<br/><br/>

:red_circle: :yellow_circle: :green_circle: :rocket: [**Latest version download**](https://github.com/s9w/starfield-navigator/releases/latest/download/starfield_navigator.zip) :rocket: :green_circle: :yellow_circle: :red_circle:

:parrot: :rabbit2: :turtle: [Changelog is here](changelog.md) :sauropod: :frog: :elephant:

(If you get a dll error, you're missing a recent [C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe))

<br/><br/>

## Insights
A couple of insights were gained from the data:
- Of the "over 100" stars announced for the final game, 75 were visible. Seven were labeled, three of them match real stars in position, name and other properties
- The game uses [Gliese numbers](https://en.wikipedia.org/wiki/Gliese_Catalogue_of_Nearby_Stars) and the known systems had correct relations to each other. Therefore it's likely that all stars are real, some of them renamed
- Over ten other unlabeled systems have been identified
- The systems with unknown labels (NARION, VOLII, CHEYENNE, JAFFA) are renames of real systems


- The reconstructed camera path showed no vertical movement. It's possible that star map navigation is constrained to left/right and front/back, just like it looked in the footage
- Of the 7 named systems in the trailer, the 5 leftmost ones had their name on the right side of the planet. The rightmost ones had their name on the left side. Barring other explanations, this might imply that the horizontal center of the map is somewhere between JAFFA and VOLII

The visible map is mostly to the right of SOL. In [this video](https://www.youtube.com/watch?v=xaNwtw7bhyk), it says "Our game is set [...] in an area that extends outward from our solar system for approximately 50 light years". That weakly implies that sol is somewhat centered. This collides with the observed fact that we already saw stars pretty far out of a 50 LY sphere around sol. But while the video is official, it's pretty questionable overall: The video animation doesn't match what the spoken description. Also the settled systems are called an "area of **our** solar system", which almost certainly is not correct.
