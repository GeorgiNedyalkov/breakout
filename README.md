# Breakout

Learn C by making breakout.

## Levels

Based on input from Claude.

Level is represented as a grid of rows and columns.
Letters are used to determine the type of brick.
The grid can only go to the middle of the screen.
The screen dimensions are fixed for now but later the game can be resized to full screen. Then they have to be in a certain aspect ration maybe?

On game start

Paddle and ball are on their starting positions.
Bricks are initialized and loaded on the level.
The remaining bricks are set.
Once the remaining bricks are 0, the level is completed.
The screen shows "Level n" is completed and "Press Space to start".
