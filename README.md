# Pika desktop companion

A small C++ Windows companion using the selected mint-green woodland pixel art.
The project folder remains Pikachu-Pet; the character and asset folder remain Pika.

## Run

Open `build/Release/PikachuPet.exe`. Keep the adjacent `data` and
`assets/pika/moods` folders with the executable.

- Pika starts in calm companion mode. Start the optional eleven-mood demo from
  settings; each mood contains four sprite frames.
- Click Pika for the playful spark reaction; drag it for smooth live movement.
- Use the sliders button on Pika's right for size, demo, quiet mode, voice,
  monitor switching or close. Four pet sizes are available; 80% is the default.
- Answer the two buttons to explore the local conversation. An unanswered
  question stays in place instead of being replaced by a timer.
- Voice starts off. Companion mode naps after five minutes of inactivity.

![Pika mood sheet](assets/pika/qa/all-moods-contact-sheet.png)

[Watch the combined loop](assets/pika/qa/all-moods-loop-large.gif) ·
[Compare with the reference](assets/pika/qa/reference-comparison.png)

## Build and validate

Requires CMake 3.20+, Visual Studio 2022 C++ tools and Windows SDK.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\PikachuPet.exe --validate
.\build\Release\PikachuPet.exe --self-test
.\build\Release\PikachuPet.exe --render
```

The runtime uses Windows libraries only. Asset preparation uses Node.js and
Sharp: `node tools/prepare-pika.cjs`, then `node tools/validate-pika.cjs`.
Install Sharp in your development environment or set PIKA_SHARP_PATH to an
existing Sharp module directory. These tools process generated artwork, not poses
drawn by code. Re-preparation resets the manifest's visual-review status.

## Documents

- [Specification](docs/PROJECT_SPEC.md)
- [Product brief](docs/PRODUCT_BRIEF.md)
- [Technical plan](docs/TECHNICAL_PLAN.md)
- [Delivery plan](docs/DELIVERY_PLAN.md)
- [Open decisions](docs/OPEN_DECISIONS.md)
- [Validation](docs/VALIDATION.md)
- [Agent instructions](AGENTS.md)
- [Pixel-art asset guide](assets/pika/README.md)
- [Generation brief](assets/pika/GENERATION_PROMPT.md)

## Still planned

Session presets, tray hide/restore, persistent preferences, richer bounded
movement, full-screen interruption suppression, mixed-DPI polish and long-session testing.
The sprite loops have four poses each; subtle texture variation remains.
