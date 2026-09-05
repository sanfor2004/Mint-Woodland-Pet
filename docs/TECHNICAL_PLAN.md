# Technical plan

## Current implementation

C++17, CMake, Win32, GDI+, Windows SAPI and a synthesized glass-tap sound.
A transparent topmost tool window hosts the pet and hand-drawn conversation UI.
The CHARACTER is now loaded from generated PNG strips, not GDI+ body shapes.

The window composition follows assets/mint-woodland-pet/reference/ui-reference.png at
675x435. The pet supports 230, 266, 300 and 333 px nearest-neighbor destination
areas, with 266 px as the default and 333 px as the reference preset. The
question, vertically stacked answers and sliders button use the reference's
measured positions, plum/white/dark-teal palette and 8 px radii. The settings
button retains the existing six menu actions.

WM_MOUSEMOVE positions the window from the original drag point, providing live
movement without accumulating offset. Companion mode is the startup default.
Timer prompts run only from the rest node, so an active question is not replaced
while the user is deciding. Quiet mode removes conversation controls from both
painting and the clickable window region.

The draggable region follows the measured union of non-transparent pixels from
all 44 frames with a 10 px margin. Dragging remains unclamped until release to
avoid monitor-boundary jumps. Release and monitor changes clamp visible content
to rcWork, allowing Mint's visible feet to sit directly above the taskbar even
though the transparent backing window extends lower. Per-monitor DPI awareness
v2 keeps coordinates stable when crossing displays with different scaling.

Rounded control fills use solid edge pixels because the native window removes a
magenta color key for transparency. Blending those edges against the key causes
a visible pink outline on light desktops; text and icon strokes remain smoothed.

All eleven strips load once from assets/mint-woodland-pet/moods beside the executable.
Each strip must be 384x96, containing four 96x96 cells. Rendering uses integer
placement and nearest-neighbor interpolation at the selected size.
Quiet mode holds frame zero. Frames advance from elapsed time, independently
of render rate. A golden spark hop preserves the earlier petting interaction.

CMake copies data and mood assets beside the executable. Keep both folders when
moving a build. Missing/invalid sprites produce a descriptive startup error.

## Asset preparation

tools/prepare-mint-woodland-pet.cjs uses Node.js and Sharp. The original visual poses come
from the built-in image generator, with the chosen reference attached.

The preparation tool extracts four frames from each 2x2 source sheet, locates
clean vertical separators, removes the magenta staging background, checks
bounds, uses fixed-scale nearest-neighbor sampling and translation-only
registration, then writes PNG strips and looping GIF previews.

Manifest schema 2 records sources, paths, frame sizes, delays and review state.
It documents the asset contract; the current C++ renderer uses the fixed paths
and synchronized duration constants rather than parsing the manifest.
A later refactor should make this data-driven.

## Dialogue

Local JSON maps IDs to mood, lines and two answers. Twenty opening variants and
thirteen reachable nodes cover good/bad branches, calm, water and food.
The deliberately narrow parser accepts strings, arrays and objects. Use literal
UTF-8 rather than escaped Unicode in dialogue.

## Framework options

Qt Quick with a C++ core remains a possible next step for richer settings,
animation controls and accessibility. Qt Widgets is an alternative for strictly
C++ UI code. No Qt dependency was added.

- https://doc.qt.io/qt-6/qml-qtquick-window.html
- https://doc.qt.io/qt-6/qtquick-window-example.html
- https://www.sfml-dev.org/

## Next technical work

Extract window, renderer, behavior, dialogue and audio modules; persist settings;
implement tray recovery; validate mixed DPI and full-screen behavior; profile
long sessions. Topmost behavior applies to ordinary desktop windows, not secure
desktops or a guarantee over exclusive full-screen applications.
