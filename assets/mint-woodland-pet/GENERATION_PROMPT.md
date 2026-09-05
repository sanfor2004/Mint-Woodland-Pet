# Mint Woodland Pet animation — reusable generation brief

The user selected the original woodland creature as Mint Woodland Pet.
Attach C:/www/Mint-Woodland-Pet/assets/mint-woodland-pet/reference/mint-woodland-pet-original.png as an actual
reference image. Do not attach the archived yellow character.

## Master prompt

Create a coherent pixel-art animation set of this same mint-green woodland pet.
Preserve the pear-shaped moss body, veined leaf ears, peach flower on the
viewer's left, glossy plum eyes, cream belly, short mitten paws, three-toed feet,
curled fern tail on the viewer's right, deep teal outlines and sage shading.
Keep the same cute proportions, face and coarse pixel style in every mood.

Create each mood as its own four-frame 2x2 sheet, read left to right, top to
bottom. Full body in every cell, same scale and baseline, generous empty margins.
Keep all ears, tail, props and small golden effects inside the cells. No labels,
dividers or UI. Four distinct coherent poses; last frame returns toward first.

Prefer real transparent RGBA output. If the generator cannot supply genuine
alpha and returns a painted checkerboard, reject that source and use a uniform
magenta #FF00FF staging background for deterministic extraction. No gradients,
shadows or background texture. Keep the sprite opaque with crisp square edges.

Generate idle first, compare it with the reference, then generate the remaining
moods with the same reference. Preserve raw candidates and exact prompts.

| ID | Action |
| --- | --- |
| idle | Neutral, inhale/ear lift, blink, return |
| look | Center, glance left, center, glance right |
| wave | Paw ready, lift, wave outward, lower |
| happy | Smile, clasp paws, arms spread delighted, return |
| sad | Ears droop, eyes lower, tear, gentle recovery |
| angry | Brow furrows, paws tense, small stomp, grumpy return |
| sleep | Seated rest, inhale, exhale, return; closed eyes |
| eat | Hold red berry, lift to mouth, nibble, lower |
| drink | Hold blue water glass, lift rim, sip, lower |
| thirsty | Empty glass by foot, reach/tap, glance at user, return |
| thunder | Gather golden sparks, raise arms, spread arms/bolts, lower |

In happy/thunder keep the torso size constant. The runtime/packaging can supply
a small positional hop. Do not make the airborne frame a smaller character.

## Review loop

Generate → compare with the original → inspect all frames → review transitions
and repeating preview → record a concrete defect → revise only the failing mood.
Check identity, silhouette, palette, pixel scale, flower/tail side, props, actual
action, frame count, margins, transparency and last-to-first continuity.
Do not duplicate one still to claim a completed animation. Do not stretch or
independently resize frames to hide inconsistent generation. If a defect repeats
three times, record it and report the limitation instead of declaring perfection.

## Save locations

All paths are under C:/www/Mint-Woodland-Pet/assets/mint-woodland-pet/.

- reference/mint-woodland-pet-original.png: selected reference, preserved.
- sources/<id>/woodland-NN.png: raw candidates.
- sources/<id>/woodland-NN-prompt.txt: exact generation prompts.
- moods/<id>/frame-00.png through frame-03.png: 96x96 RGBA.
- moods/<id>/strip.png: 384x96 horizontal strip.
- qa/<id>-loop.gif: one infinite loop per mood.
- qa/all-moods-loop.gif and all-moods-loop-large.gif: two cycles per mood.
- qa/all-moods-contact-sheet.png, all-frames.png, reference-comparison.png.
- qa/review.md and geometry.json: findings and deterministic checks.
- manifest.json: selected sources, frame paths, durations and review state.

Copy returned generated files into the project; do not rely on a generator
accepting arbitrary output paths. Preserve unsuccessful attempts separately.

## Runtime and verification

Use nearest-neighbor scaling and integer placement. Default timing: 240 ms/frame,
sleep 450 ms, thunder 160 ms. Keep C++ timing synchronized with the manifest.
Run the asset validator, build the application, run --validate, --self-test and
--render, inspect resulting images and report the limits of desktop testing.

This brief has been executed for the woodland design. See manifest.json and
qa/review.md for selected outputs; historical earlier character failures are
not the current status.
