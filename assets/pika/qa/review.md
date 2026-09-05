# Woodland Pika visual review — 2026-09-04

## Reference and selected outputs

The active reference is reference/pika-original.png, copied from the original
woodland concept chosen by the user. The old yellow character reference was
preserved in reference/archive/pikachu-original.png.

Eleven moods / 44 frames are prepared and integrated. Selected raw sources:
woodland-02.png for idle, happy and thunder; woodland-01.png for the others.
Exact prompts are next to each source.

## Iterations

1. Idle woodland-01: rejected. Painted checkerboard instead of alpha and cramped
   horizontal spacing. Replaced with a separated 2x2 sheet on staging magenta.
2. Idle woodland-02: same face/leaf ears/flower/fern tail; background can be
   removed deterministically. Reviewed for prototype use.
3. Happy/thunder woodland-01: smaller bodies in jumping frames. Rejected for
   size drift and regenerated with planted poses and constant body scale.
4. Happy/thunder woodland-02: steadier silhouettes; small positional hops are
   applied in packaging/runtime instead. The extraction tool finds the actual
   empty separator to avoid cutting a tail near the nominal cell boundary.

## Per-mood observations

| Mood | Observation |
| --- | --- |
| idle | Same selected features; breathing/expression changes and closed-eye frame |
| look | Directional eye/head changes visible; flower and tail retain their sides |
| wave | Paw rises and moves outward; friendly face preserved |
| happy | Clasp/open paws and smile; revised frame avoids conspicuous shrinking |
| sad | Drooping leaf ears and tears read clearly |
| angry | Furrowed brows and expressive stomp effects; stays cute |
| sleep | Low seated pose, relaxed leaf ears, closed eyes and sleep marks |
| eat | Berry moves to mouth and is bitten; both paws participate |
| drink | Blue glass raises to mouth with closed-eye sip |
| thirsty | Empty glass remains beside the foot; paw reaches to tap |
| thunder | Golden spark effects and raised arms; revised torso size steadier |

## Checks and qualifications

Inspected original reference, source sheets, all-frames sheet, contact sheet,
reference comparison and a native rendered scene. The set reads as one woodland
character with matching main features. Fine foliage/shading details vary between
frames, and these are four-pose loops rather than high-frame-count animation.
The flower and body proportions are not pixel-identical to the concept image.

Geometry/alpha validation passes for all 44 frames. GIFs have verified frame
counts, delays and infinite-loop metadata. Ordered frame review checks the loop
endpoints; hands-on playback/desktop feel remains available for user review.

Status: reviewed for this prototype, with the above minor art limitations.
No claim of user approval of every generated frame or legal clearance is made.
