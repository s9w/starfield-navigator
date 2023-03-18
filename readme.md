# Starfield Navigator

On 2022-06-12, Bethesda released the [first ingame footage](https://www.youtube.com/watch?v=zmb2FJGvnAw) of **Starfield**. Around 13:30 in the video, the starmap is visible for ~6 seconds. That scene was used to reconstruct parts of the ingame starmap. The **Starfield Navigator** is a small program uses that data in a couple of ways. This site contains the source of the program, a downloadable version as well as discussion of the method, current efforts and findings.

https://user-images.githubusercontent.com/6044318/177479550-c8bf36b0-9571-4ade-a45a-d1f4a05bc733.mp4

## Launch Date Announcement Video
In the [Launch Date Announcement](https://www.youtube.com/watch?v=raWbElTCea8) on 2023-03-08, two more existing star names could be seen: *Tau Ceti* and *Cassiopeiae*. "Cassiopeiae" is a huge star constellation and we can't know which one they refer to. Therefore from v0.14 on, the most likely (i.e. close and bright) stars of Cassiopeiae as well as Tau Ceti are now added to the navigator as **yellow entries**.

## Download

:arrow_down: :floppy_disk: [**Download current version**](https://github.com/s9w/starfield-navigator/releases/latest/download/starfield_navigator.zip) :floppy_disk: :arrow_down:

:parrot: :rabbit2: :turtle: [Changelog is here](changelog.md) :sauropod: :frog: :elephant:

(If you get a dll error, you're missing a recent [C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe))

## Usage
- In the (default) *frontal* camera mode, use WASD to mov along the map
- The *center selection* mode centers the selected System. Selection can be done with the *System selector*. You can pivot around the system with right mouse button or WASD
- The galactic center mode is the same, but in [galactic coordinates](https://en.wikipedia.org/wiki/Galactic_coordinate_system)
- The reveal mode replays the scene from the [official Gameplay Reveal](https://youtu.be/zmb2FJGvnAw?t=810). This is not perfect as the aspect ratio is not strictly controlled

Also note the tool window in the upper right. It can show connections between systems with different jump ranges. It can also calculate the optimal route between two system and show the individual jumps.

## Details and analysis
In the first showcase, we get a ~6 second shot of the camera moving in the starmap. With 3D Tracking software, the location of all 75 visible stars was extracted. These positions are arbitrary and only correct in relation to each other. Of the stars that had names, there were three stars that exist IRL: Sol, Alpha Centauri and Porrima. The *real* position of those stars was used to align those arbitrary reconstructed positions to their actual coordinates. In three dimensions, three positions are luckily enough. It was then discovered that the positions of the other (unlabelled) stars also seem to match known stars. At that point, publicly accessible star catalogues were used to programmatically find the most probably ID for all the stars visible in the showcase. For most stars, there's a very good match. With those good matches in hand, original estimates for the FOV of the camera footage and other assumptions could be used to further calibrate the measurements so that all stars could be matched to an error of about 0.1 ly.

There are some astronomically tricky parts left: We know for example that they don't always use realistic data for multiple-star systems. Particularly, Alpha Centauri was shown as a single-star system. Yet there are systems like Xi Scorpii that are quintuple with hugely complex mechanics which was visible as two stars. Also they seem to be optimistic about the number of planets in systems, at least compared to current findings.

With 100% of the stars identified, some rough analysis is possible. In the reconstructed parts of the universe, not all known stars were included. A first guess to what was considered good candidates might be brightness, usually inversely quantified by "absolute magnitude". If we limit the analysis to stars closer than 150 ly (that's the furthest reconstructed star) and further than 30 ly (near-earth stars might have story priority), we can plot a histogram of what stars exist and were used:

![magnitude_analysis](https://user-images.githubusercontent.com/6044318/178492776-1e03e154-78f6-4fc3-b839-d6bc883c482c.png)

That suggests a preference for low magnitudes/bright stars. If we look closer to earth, me may have a clue to another factor: Both Gliese 667 and Wolf 359 are included. They're really dim stars but have notoriety (Star Trek and Alien vs Predator).

A star that would fit all three criteria is **Epsilon Eridani**. It's close to earth, is popular in fiction and reality, and while not particularly bright - is of the most popular brightness otherwise included. Looking at the place where Epsilon Eridani is expected, we can see nothing: It's occluded by some interface element in the showcase. This may be one of the safer bets for stars that might be in the game.