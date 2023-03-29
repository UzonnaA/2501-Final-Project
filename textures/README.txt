Here's what I got for A4.

1a: Pressing the "F" key spawns a bullet, which will fire straight and fast
in the direction the player faces. Not super complicated, just rotate the bullet
as needed and change the vecloity scaling to something higher than the player.

1b: GameObject has member variables that allow me to force the object to die after 5 seconds
if I want it to. There's also a new Kill function which makes some future stuff easier.

1c: Copied from a previous assignment.

1d: There is a whole new function that handles ray collsion and calculates time frame so
the bullet doesn't disappear immediately. A lot of math and calculations. Check out the function RayCollision() for details, but everything looks good in game.

2: Each bullet has a seperate particle system which I changed to have a green hue and be thinner/longer than the original.

3: ... (So long, 15% of my grade)

4: There's a blade sprite whose parent I set to the player. If a GameObject is a child, its transformation function changes
to be based on its parent (So in the game loop I set the blade position to 0,0,0 which actually sets it to the player's current position). Finally, I re-used the code for turning the player to allow any GameObject of "blade" type to rotate itself.

Comments can be seen throughout.

Final Note: https://www.youtube.com/watch?v=XEunwF8JDSw
In case, there's a crash I posted a YT video.
That said, if there is a crash, just try one more time.

Developed on Windows 11.
