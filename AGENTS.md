# Project instructions

## Current user decisions

Build a small C++ Windows desktop companion named Mint Woodland Pet. Keep the
existing Pikachu-Pet repository folder for continuity, while using the active
assets/mint-woodland-pet folder and Mint naming in product-facing text and code.

The user selected the ORIGINAL woodland creature as the replacement character.
Active reference: assets/mint-woodland-pet/reference/mint-woodland-pet-original.png (mint body, leaf ears,
peach flower, cream belly, plum eyes, fern tail). The older yellow character
reference is archived at reference/archive/pikachu-original.png and must not be
used as the current generation reference.

Read README.md and docs/PROJECT_SPEC.md before changing behavior.

## Maintaining requirements

Whenever the user supplies a material requirement, update PROJECT_SPEC.md, the
relevant technical/delivery/open-decision documents, and this file's persistent
rules or decision summary. Latest explicit user instructions override old notes.
Distinguish requested, implemented and verified work. Preserve unrelated changes.

## Art and asset rules

- Keep all active references, generated sources, mood sprites and QA under assets/mint-woodland-pet.
- Preserve the chosen character's leaf ears, peach flower on viewer-left, cream
  belly, glossy plum eyes, fern tail on viewer-right and mint/sage palette.
- Generate, visually compare with the reference, inspect frame transitions and
  revise specific defects. The user explicitly requested repeated review.
- Required mood IDs: idle, look, wave, happy, sad, angry, sleep, eat, drink,
  thirsty, thunder. The last remains a playful golden spark reaction.
- Runtime contract: four 96x96 RGBA frames and a 384x96 strip per mood.
- Keep raw generated sources and exact prompts. Never substitute duplicated
  stills for a missing action. Do not claim pixel-perfect identity.
- Use nearest-neighbor rendering and integer placement.
- Do not claim AI artwork has automatic legal clearance or CC0 status.

## Product rules

C++ remains the language direction. Qt Quick is optional future work, not an
installed dependency. Follow assets/mint-woodland-pet/reference/ui-reference.png: compact
white question bubble with plum monospace text, two vertically stacked dark-teal
answers and an 8 px rounded sliders/settings button to the pet's right. Keep short
voice, quiet mode, idle sleep, petting and restrained movement. Conversation
content lives in JSON, including twenty greetings and good/bad branches. Do not
record typing or application content. New online or microphone features need a
product decision.

Start in companion mode. Keep unanswered questions stable, collapse short rest
messages after twelve seconds, and restrict automatic check-ins to the rest
state. Demo mode is user-initiated. Dragging must move the window continuously.
Offer 230, 266, 300 and 333 px pet sizes with 266 px as the default. Keep the
drag region close to visible sprite pixels, move freely while held, and clamp
visible content rather than the transparent canvas at monitor work-area edges.

The runtime uses magenta color-key transparency. Keep rounded UI surface edges
unblended so magenta does not appear as a pink fringe on the desktop.

## Verification

Build C++ after changes. Run --validate, --self-test and --render for relevant
changes. Use tools/validate-mint-woodland-pet.cjs after asset or dialogue changes.
After generation, inspect reference-comparison.png, all-frames.png and loop
previews. Preparation resets visual status: perform a fresh review before marking
assets reviewed. Report native desktop observations separately from handler tests.
Do not confuse the JSON asset manifest with a runtime loader: current runtime
loads fixed mood strip paths; keep durations synchronized until that is refactored.

## Decision history

- 2026-09-05: User supplied morning-pixel-art and retro-title references for a
  GitHub and external marketing banner. Saved both under reference/poster and created a wide happy
  morning hero with Mint on the left looking toward the title on the right.
- 2026-09-05: User requested a smaller pet, a smaller mouse target, smoother
  right-screen movement and placement against the taskbar. Added four size
  presets, visible-sprite hit bounds, release-time clamping and per-monitor DPI v2.
- 2026-09-05: User requested smoother dragging and calmer behavior. Implemented
  live pointer-following drag, companion-mode startup, stable active questions,
  collapsing rest UI, an eight-second demo cadence and longer prompt intervals.
- 2026-09-05: User supplied a 675x435 UI reference. Saved it as
  assets/mint-woodland-pet/reference/ui-reference.png and matched its question, stacked
  answers, sliders button, spacing, palette and 8 px corner treatment.
- 2026-09-04: User described a quiet interactive desktop pet and selected C++.
- 2026-09-04: User requested pixel art, eleven moods, JSON dialogue and iterative QA.
- 2026-09-04: Earlier franchise-art generation attempts were blocked; historical
  prompts/errors remain in sources. Those failures are not the current asset status.
- 2026-09-05: User renamed the active product to Mint Woodland Pet. The repository
  folder remains Pikachu-Pet, while active code, assets and documentation use Mint.
- 2026-09-04: User chose the generated woodland reference and authorized all mood generation and continuation. Produced 44 frames and
  integrated eleven strips. Revised idle transparency/layout and happy/thunder
  size drift. Build, asset validation and native-handler smoke checks pass.
