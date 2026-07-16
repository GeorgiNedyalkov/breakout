# Breakout

Learn C by making breakout.


## Bugs:

- [] Center the text
- [] Fix side collision
- [] Empty/undefined levels create garbage-colored bricks
- [] `level_index` bounds check is off-by-one

## How to Fix

### Empty/undefined levels create garbage-colored bricks

`levels` is sized for `MAX_LEVELS` entries but only one level string is actually
given, so every level past index 0 is zero-filled (`'\0'`) instead of `'.'`.
In `init_level`, the character check only handles `'.'`, `'b'`, and `'v'` —
anything else falls into the `else` branch and builds a `brick` without ever
setting `.color` or `.lives`, so it renders with whatever was already on the
stack.

Two things need to change:
1. Give `init_level` a real default case: any character that isn't a
   recognized brick type should be treated the same as `'.'` (skipped), not
   silently fall through into brick creation.
2. Decide what "no more levels defined" should mean for the game (loop back
   to level 0? show a win screen?), and make `init_level` handle that case
   explicitly instead of reading a level that's just zero bytes.

### `level_index` bounds check is off-by-one

Valid indices into `levels` are `0` through `MAX_LEVELS - 1`. The current
check `if (level_index > MAX_LEVELS) return;` only rejects indices strictly
greater than `MAX_LEVELS`, so `level_index == MAX_LEVELS` slips through and
reads one row past the end of the array — undefined behavior.

The comparison should be `>=` instead of `>`. This ties directly into the
previous bug: once you've decided what happens after the last level, this is
the check that should enforce it.

## Collission and Game Feel

The fun thing for Breakout is the ability to control the ball if you hit it in the right place.
Right now the ball just changes its y direction. We need to use a function that tracks the distance
from the paddle to where it hit the ball in order to influence its direction.

Also the collision on the side is wrong.

Brick collisions.

See of the paddle collides from the side then reverse x.
See of the paddle collides from the top then reverse  y.

##
