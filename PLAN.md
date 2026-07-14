# Plan: Ball Physics + Levels

Notes for tomorrow's session. Logic and math only — no code written here, figure out the C on the day. Order below is the suggested build order: each step is testable in isolation before moving to the next.

## 1. Side-aware collision (do this first — it's a prerequisite building block)

Currently every collision (paddle and bricks) always flips `direction.y`, regardless of which face was hit.

- For a given ball/rectangle collision, compute the overlap depth on each axis separately:
  - x-overlap = (smaller of the two right edges) minus (larger of the two left edges)
  - y-overlap = (smaller of the two bottom edges) minus (larger of the two top edges)
- Whichever axis has the **smaller** overlap is the one that was just crossed — flip that axis's direction sign only.
- Apply this to brick collisions first (bricks are the case where it matters most once layouts aren't a single row).
- Test: arrange a couple of bricks so the ball can clearly approach from the side vs. from below, confirm it bounces off the correct face in each case.

## 2. Paddle bounce angle

Replace the flat "always flip y" paddle response with position-based aiming.

- On paddle hit, compute: (ball center x − paddle center x) ÷ (half paddle width). This gives a ratio from -1 (far left edge) to +1 (far right edge), 0 at dead center.
- Clamp that ratio to [-1, 1] in case the collision was detected slightly past the paddle's edge.
- Set the new horizontal direction to that ratio; vertical direction to a fixed "upward" value.
- Normalize the resulting direction vector (divide both components by the vector's magnitude) so ball speed stays constant regardless of where it was hit — otherwise edge hits will send the ball faster than center hits.
- Test: hit the paddle at the far left, center, far right — confirm the ball's outgoing angle visibly changes and travel speed looks consistent across all three.

## 3. Levels — data-driven brick layout

Goal: replace the single hardcoded row of bricks with a system that can load different layouts.

- Design a level as a grid: rows × columns, where each cell indicates "no brick" or "brick present" (and eventually brick toughness — reuse the existing `lives` field per brick for this).
- Brick pixel position becomes derived from its row/column index rather than a flat loop counter: x from column × (brick width + gap), y from some top offset + row × (brick height + gap).
- Size brick storage for the largest layout you intend to support; each level's grid determines which of those slots are actually active. Keep using the existing `is_dead` flag and remaining-count tracking per level — that logic doesn't need to change, it just needs to operate on a level's active bricks instead of the whole array.
- Define at least 2 levels' worth of grid data to prove the system generalizes (not just the current single row).

## 4. Level progression / game state

- Add a current-level index to the `game` struct (currently just holds `mode`).
- On reaching COMPLETED: instead of sitting there permanently, provide a way to advance — increment the level index, reset ball and paddle to starting positions, rebuild the brick grid from the next level's data, drop mode back to START.
- If the level index goes past the last defined level, treat that as a distinct "all levels cleared" end state, separate from the per-level COMPLETED screen.
- Test: clear a level, confirm the next layout loads and ball/paddle reset correctly; clear the last level, confirm the distinct end state shows instead of trying to load a nonexistent next level.

## 5. Later / optional

- Per-level difficulty knobs (ball speed multiplier, brick toughness) stored alongside each level's grid data.
- Wire up `collision_sound` (already loaded, currently unused) to play on paddle/brick hits — natural to add once side-aware collision is in place, since you'll already be touching both collision call sites.
- Restart handling for the OVER state (currently no way out once lives hit 0).
