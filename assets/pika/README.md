# Pika pixel-art assets

Active reference: [reference/pika-original.png](reference/pika-original.png).
It is the user-selected original mint woodland character. The Pika name and
folder remain unchanged.

## Contents

- reference/: active woodland reference; earlier yellow artwork under archive/.
- sources/: raw generated mood sheets and the exact prompts. Old failed attempts
  remain as historical evidence; selected sources are listed in manifest.json.
- moods/: eleven folders, each with four 96x96 RGBA PNG frames and a 384x96 strip.
- qa/: contact sheets, all 44 frames, reference comparison, individual GIF loops,
  combined loops, geometry checks and visual findings.
- manifest.json: current asset paths, timings and review status.
- GENERATION_PROMPT.md: updated reusable woodland-character brief.

![All moods](qa/all-moods-contact-sheet.png)

[Large looping preview](qa/all-moods-loop-large.gif) ·
[Reference comparison](qa/reference-comparison.png) · [Review notes](qa/review.md)

Moods: idle, look, wave, happy, sad, angry, sleep, eat, drink, thirsty, thunder.
Thunder is a playful golden spark reaction.

## Processing and review

The built-in image generator created each mood, with the woodland reference
attached. First idle output used a painted checkerboard and was rejected.
A flat magenta staging background allowed deterministic transparent extraction.
Happy/thunder were regenerated to remove jump-related size shrinkage.

tools/prepare-pika.cjs extracts and registers the artwork; it does not generate
poses. tools/validate-pika.cjs verifies dimensions, alpha, bounds, distinct frames,
loop metadata and dialogue links. Use Node.js plus Sharp, optionally resolved
through PIKA_SHARP_PATH. Preparation resets visual status for a fresh review.

The runtime has been updated to use these strips. The generated design keeps the
selected character's features with small shading variations; it is not an exact
pixel-for-pixel reproduction. The selected concept's earlier location under
assets/original-mascot is historical and is not used by the application.
