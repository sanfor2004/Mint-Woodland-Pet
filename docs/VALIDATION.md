# Validation — Mint Woodland Pet sprite build

Checked 2026-09-05.

## Passed

- Release C++ build with Visual Studio 2022 and Windows SDK.
- --validate: all eleven sprite strips load with correct dimensions; all
  conversation moods exist and twenty greeting variants are present.
- --self-test: actual window-message handlers exercise good/bad answers,
  drinking, eating, petting, quiet toggling and demo restart without global input.
- --render: native renderer exports eleven scene previews and the contact sheet.
- tools/validate-mint-woodland-pet.cjs: 44 nonempty 96x96 RGBA frames; safe margins; binary alpha;
  no residual staging chroma; at least three distinct images per mood; eleven
  384x96 strips; eleven four-frame infinite GIF loops; an 88-frame combined loop;
  thirteen reachable dialogue nodes and twenty greetings.
- Visual inspection of generated source frames, combined frame sheets, desktop
  rendering and reference comparison. Specific revisions corrected the first
  idle sheet's fake transparency/layout and happy/thunder jumping size drift.
- Rebuilt the 675x435 conversation scene from the supplied UI reference and
  compared the saved reference with the native GDI+ render. Control placement,
  palette, stacking, corner radii, pet scale and settings icon were checked.
- Removed color-key blending from rounded control edges so question, answer and
  settings surfaces do not retain pink transparency-key pixels.
- Rebuilt and reran configuration, message-handler and render checks after the
  live-drag and calm-cadence behavior update.
- Native handler self-test verifies smaller/larger controls and confirms that
  visible content reaches the bottom-right edges of the monitor work area.
- Rendered the longest shipped dialogue line in `long-question-render.png` and
  visually confirmed that its complete 50-character question wraps to two lines
  inside the unchanged 235x42 bubble without clipping.

## Limits of verification

Image files and ordered frame sheets were inspected; automated GIF checks verify
frame count, timing metadata and infinite-loop configuration. This is not an
eight-hour or full native-desktop behavioral test. Fine pixel shading varies
between generated frames; further art polish can follow user feedback.

Hands-on audio, monitor switching, full-screen apps, mixed DPI and long-session
CPU/memory measurements remain outstanding.

## Build environment note

The tool environment exposes duplicate Path/PATH variables. Build invocations
normalized child-process environment keys to avoid MSBuild's duplicate-key error.
No system environment settings were changed.
