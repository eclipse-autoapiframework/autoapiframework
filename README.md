# Blueprint projects around the Eclipse Automotive API Framework

This repository includes the following blueprint projects in the examples folder: 

| Folder | Project description | 
| ------ | ------------------- | 
| [bp-vss-vaf](./bp-vss-vaf/) | This project demonstrates the combined usage of the [VSS catalogue editing tool](https://github.com/eclipse-autoapiframework/vss-gui-tool) and the [Application Framework](https://github.com/eclipse-autoapiframework/application-framework). The scenario for this example is a simple speed hazard detection application. | 
| [bp-ifex-vaf](./bp-ifex-vaf/) | This project extends the previous blueprint by exchanging VSS with the [Interface Exchange (IFEX)](https://covesa.global/project/interface-exchange/) format for the representation of the signal catalogue. |

More projects to follow...

## Documentation Generation

This repository supports documentation generation using the [Eclipse S-CORE Docs-As-Code](https://eclipse-score.github.io/docs-as-code) approach. For more information and guidance, visit the official docs-as-code site.

## Dev Container Usage

This repository provides a pre-configured development container [Eclipse S-CORE DevContainer](https://github.com/eclipse-score/devcontainer) for a consistent and reproducible development environment. To use the dev container:

1. Open the project in Visual Studio Code.
2. Install the "Dev Containers" extension if not already installed.
3. Reopen the workspace in the container when prompted.

The dev container automatically prepares the workspace, including CA bundle setup and system trust updates if configured. See `.devcontainer/prepare_workspace.sh` for details.


