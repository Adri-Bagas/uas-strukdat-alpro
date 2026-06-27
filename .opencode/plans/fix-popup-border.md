# Fix: Popup border disappears after typing finishes

## Root Cause

`Popup::render()` only draws the border during `ANIMATING` state. In `TYPING` and `WAITING` states, only text lines are drawn + `wnoutrefresh`. Since `state_stack.top()->render()` runs BEFORE `active_popup->render()` in the game loop (GameEngine.cpp:175-178), the state's render overwrites the border area on `newscr` every frame. The popup's `wnoutrefresh` never re-copies the border lines because they're not touched.

## Changes

In `src/utils/components/Popup.cpp`, `Popup::render()`:

1. **TYPING** branch: add `box(win, 0, 0)` before drawing incremental text — touches border cells without clearing text, ensuring they're copied to `newscr`.

2. **WAITING** branch: add `werase(win)` + `box(win, 0, 0)` before drawing all text — redraws border + text cleanly each frame.

## Build

```bash
nix-shell --command "cmake -B build -S . && cmake --build build" --impure
```
