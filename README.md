The Autoapiframework project is supported by Chips JU.

<img src="CHIPS logo_RGB_full_colour.png" alt="arch-module-if" width="200"/>
<img src="EN_Co-fundedbytheEU_RGB_POS.png" alt="arch-module-if" width="400"/><br>

The Shift2SDV project (Grant Agreement No. 101194245) is supported by the Chips Joint Undertaking and its members, including top-up funding by the national authorities.

<img src="Shift2sdv_logo_tagline_blue.svg" alt="arch-module-if" width="200"/><br>

# Documentation Generation

This repository supports documentation generation using the [Eclipse S-CORE Docs-As-Code](https://eclipse-score.github.io/docs-as-code) approach. For more information and guidance, visit the official docs-as-code site.

# Dev Container Usage

This repository provides a pre-configured development container [Eclipse S-CORE DevContainer](https://github.com/eclipse-score/devcontainer) for a consistent and reproducible development environment. To use the dev container:

1. Open the project in Visual Studio Code.
2. Install the "Dev Containers" extension if not already installed.
3. Edit Line 7 in file `.devcontainer/prepare_workspace.sh`. Do not check in this change.
4. Open the Command Palette (Shift + Ctrl + P) and select “Dev Containers: Rebuild and Reopen in Container”.
5. Lower left corner: `Dev Container: eclipse-s-core` should be visible.
6. Run `bazel run //:live_preview` in terminal to create the documentation in folder `_build`.

The dev container automatically prepares the workspace, including CA bundle setup and system trust updates if configured. See `.devcontainer/prepare_workspace.sh` for details.


