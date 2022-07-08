# 0.8
- In frontal view, the selected system is now indicated with a ring around the star in the main view

- ![](https://user-images.githubusercontent.com/6044318/177981869-7493a4ed-6d91-4fb8-9632-cfb394014a52.png)

- Systems with speculative names are now written in purple in the main view
- Removed two wrong identifications, added seven
- System list is now sorted according to position in Starfield (from left to right). Makes it easier to find things in the frontal view

# 0.7
- In galactic coordinate mode, lines can now be drawn down to the ecliptic of the selected planet:
![image](https://user-images.githubusercontent.com/6044318/177693717-96c296a1-ff00-4a52-8b7a-bf2d67f094dc.png)

- Hovering in system selector now shows (reconstructed) galactic coordinates:

![image](https://user-images.githubusercontent.com/6044318/177693521-921fbcb2-d50c-4bf5-9e0e-1f3991944d54.png)

# 0.6
- Added [galactic coordinate](https://en.wikipedia.org/wiki/Galactic_coordinate_system) mode. Mainly for people who want to go planet hunting
- Hovering over the names in the system selector now shows a popup with the original name, even for the unknown ones. Mainly for people who want to go planet hunting

# 0.5
- Switched star colors: Big in the reveal is green, small is red
- bugfix: wrong system name was shown in closest tab

# 0.4
- Stars are now drawn either red (was a big dot in the reveal video) or blue (was a small dot)
- System selector now properly differentiates ingame and real-life names of systems
- Fixed messed up system labels in closest star tab
- Fixed crash on minimize

# 0.3
- Fixed visual distortions when resizing a window to a non 16/9 aspect
- Internal change to galactic coordinates
- Added more likely stars
- Change to different reference data, distances might have changed slightly

# 0.2
- In the jump calculation mode, all possible connections are now shown, too
- In the closest stars mode, lines to the closest stars are now drawn
- In center view, a plane/XYZ indicator is now drawn on the active system
- The window can now be properly resized
- Closest stars always use the currently selected system
- Center camera automatically switches to the selected system (and keeps orientation, distance)
- Version is now displayed in the title
- Added tooltips about controls over camera mode buttons

- Fixed a crash in the jump calculations between very close systems
- Fixed a rendering issue where lines and system labels behind the camera were drawn, especially in center mode
- Fixed scrolling by mouse wheel not working in the GUI
- Fixed GUI not refreshing the visible connections when switching between connections and jumps

# 0.1
initial release
