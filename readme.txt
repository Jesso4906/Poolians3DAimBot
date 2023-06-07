Aim bot for: https://store.steampowered.com/app/670290/Real_Pool_3D__Poolians/

While the game is running, just run PooliansAimBotInjector.exe. To avoid crashing, you should want to wait until you are in a match.

Shots are calculated by using directions and trigonometry. First, the direction between the target ball and pocket is calculated by subtracting the pocket's coordinates from the target ball's coordinates. Next, the ghost ball, or the position the cue ball needs to hit the target ball at is calculated by multiplying the previous direction by the cue ball's radius. Then, the target direction is calculated by subtracting the cue ball's coordinates from the ghost ball's coordinates. Finally, the target angle is calculated using inverse tangent, which is then converted to degrees by multiplying by 180 and dividing by PI.

Although ricochet shots are not supported, you can perform combination shots.

Controls:
  ins - Exit
  R - reset (if angles are off)
  M - Increase number of target balls for a combination shot. Target ball 1 is the one you want to pocket.
  , - Change selected ball to edit if their are multible target balls
  0 - Toggle between selecting stripes vs solids
  1-8 - Select balls 1 - 8, or 9 - 15 if 0 is pressed
  Z-N - Select pocket where 1 is the bottom right (close to bar); going clockwise
