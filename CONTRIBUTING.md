# Contributing to Mint Woodland Pet

Thank you for helping improve Mint Woodland Pet. Small, focused pull requests
are easiest to review.

## Before opening a pull request

1. Open or find an issue describing the behavior you want to change.
2. Keep the implementation focused and preserve unrelated work.
3. Build the Release target and run the relevant checks:

   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   .\build\Release\MintWoodlandPet.exe --validate
   .\build\Release\MintWoodlandPet.exe --self-test
   .\build\Release\MintWoodlandPet.exe --render
   ```

4. Explain what changed, why it changed, and how you verified it.

## Licensing contributions

By submitting source code, build files, tools, dialogue, or documentation, you
confirm that you have the right to contribute it and agree that it may be
distributed under the repository's MIT License.

Do not include copyrighted third-party code, images, music, fonts, trademarks,
or generated material whose terms are unclear. General pull requests should not
add or replace Mint Woodland Pet character or marketing artwork. Discuss asset
contributions with the maintainer first because the artwork follows the separate
[Asset Rights Notice](ASSET_LICENSE.md).

## Style and behavior

- Use C++17 and keep Windows-specific behavior clear and bounded.
- Keep dialogue local and avoid collecting typed text or application content.
- Preserve the active character design and nearest-neighbor sprite rendering.
- Update relevant documentation when behavior, assets, or requirements change.
- Add comments when they teach intent or a non-obvious constraint.

All participation must follow the [Code of Conduct](CODE_OF_CONDUCT.md).
