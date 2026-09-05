# Mint Woodland Pet project specification

Updated 2026-09-05.

## Selected character and experience

Mint Woodland Pet is a small, friendly desktop companion. The user selected the
mint-green woodland creature as the active character. The Pikachu-Pet repository
folder remains for continuity, while code and active assets use Mint naming.

The active reference is assets/mint-woodland-pet/reference/mint-woodland-pet-original.png. Preserve its
leaf ears, peach flower on viewer-left, plum eyes, cream belly, moss-like body,
fern tail on viewer-right and golden pollen effects. The previous yellow
reference is preserved in reference/archive and is no longer active.

On launch, Mint greets in companion mode. The question remains stable until the
user answers; the mood demonstration starts only when selected in settings. Mint
rests nearby, looks around, reacts to clicks, offers short conversations and
naps when the user is idle. Petting triggers a playful golden spark reaction
and a small upward-right movement. Desired daily sessions are one to eight hours;
session presets and long-duration qualification are still pending.

## Implemented moods

| ID | Behavior |
| --- | --- |
| idle | Breathing/expression change and blink |
| look | Eyes/head glance left and right |
| wave | Paw raises, waves and lowers |
| happy | Clasped paws, delighted expression and a small hop |
| sad | Drooping ears, downcast eyes and tears |
| angry | Furrowed brows, tense paws and small expressive effects |
| sleep | Closed eyes, resting ears and breathing poses |
| eat | Holds berry, lifts it, nibbles and lowers it |
| drink | Holds a blue glass, raises it, sips and lowers it |
| thirsty | Empty glass beside feet, tap gesture and hopeful expression |
| thunder | Golden sparks, raised paws and playful movement |

Each mood has four transparent PNG frames and a horizontal strip. The source
images were generated as 2x2 sheets; deterministic extraction produces 96x96
cells without stretching. Playback is 240 ms/frame normally, 450 ms for sleep
and 160 ms for thunder. The optional desktop demo advances moods every eight seconds.

The user requested repeated visual comparison with the reference. Idle was
revised for background/layout defects; happy/thunder were revised for size drift.
The result preserves the selected design with some texture and pose variation;
it is not a pixel-for-pixel copy of the reference.

## Interface and interactions

Mint starts near the bottom right above ordinary desktop windows. The sliders
button opens settings. Speech uses a compact rounded bubble with two answer
buttons stacked on the left.

The accepted UI reference is assets/mint-woodland-pet/reference/ui-reference.png. The compact
675x435 composition uses a 235x42 white question bubble, plum monospace text,
two vertically stacked dark-teal answer buttons, 8 px corner radii and a 28x28
sliders/settings control to the character's right. The initial prompt is
"HI, how you doing?" with "Doing well" and "Feel bad" answers.

Settings menu: demo on/off, quiet mode, voice, next screen, ask a question and close.
It also provides smaller/larger controls across 230, 266, 300 and 333 px image
sizes; 266 px is the daily-use default and 333 px preserves the supplied UI
reference scale. Click Mint to pet the companion; dragging moves it continuously with the
pointer. Only the union of visible sprite pixels plus a small click margin is
draggable. The idle detector measures
time since input, and large pointer motion briefly pauses wandering. The
prototype does not inspect application contents or record typed keys.

## Conversations and sound

Local JSON holds twenty greetings and thirteen reachable dialogue nodes, with
two answers per node. Good/bad responses branch into encouragement, calm,
company, breaks, food and water. Sad, eating and drinking dialogue nodes select
their corresponding sprites.

Windows system speech is optional and initially off. A synthesized glass tap
accompanies thirst. Quiet mode suppresses sound and unsolicited prompts and
hides the question/answer controls.
No AI service, account, microphone input or long-term personal memory is required.

## Prototype timing

Idle sleep: five minutes. Active check-in: about fifteen minutes. Thirst: about
forty-five minutes. Short rest messages collapse after twelve seconds. Busy
pointer suppression: two seconds. These are prototype defaults.

## Remaining daily-use work

One/eight-hour session controls; tray hide/restore; saved preferences; explicit
bubble dismissal; full-screen suppression; accessibility;
mixed-DPI and monitor-change checks; CPU/memory measurement and long-session tests.
Four-frame art may benefit from additional in-between poses after user review.

## Latest decision record

2026-09-05: The software was prepared for public open-source contribution under
MIT, with Copyright © 2026 sanfor2004 retained. Mint character and marketing art
remain separately protected; third-party historical and poster references are
excluded from both licenses. Community, contribution, support, and security
files document the public repository workflow.

2026-09-05: Added a reusable GitHub and external marketing banner based on two user-supplied visual references.
The final wide pixel-art composition shows Mint on the left in a happy morning
woodland, looking curiously right toward the exact Mint Woodland Pet title, with
a soft ground shadow. The source references are retained under
assets/mint-woodland-pet/reference/poster.

2026-09-05: Companion mode became the default, live dragging was implemented,
the optional demo was slowed, active questions were protected from timed
replacement, and rest UI now collapses automatically.

2026-09-05: User supplied and approved a compact UI reference. Saved the image
under assets/mint-woodland-pet/reference, matched the runtime composition and settings icon,
and synchronized answer hit targets and render previews with the new layout.

2026-09-05: Active product renamed Mint Woodland Pet. The repository remains
Pikachu-Pet while code, active assets and documentation use Mint naming.

2026-09-04: Original woodland reference accepted; eleven moods generated and
integrated. See docs/VALIDATION.md and assets/mint-woodland-pet/qa/review.md.
