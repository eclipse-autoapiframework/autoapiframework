from pathlib import Path

from vaf import save_part_of_main_model
from vaf.vafgeneration import generate_cac_support
from vaf.core.common.utils import ProjectType

# Import the interface project model
from . import interfaces

def export_model(skip_validation: bool = False):
    script_path = Path(__file__).resolve().parent
    export_path = script_path / "export"
    export_path.mkdir(parents=True, exist_ok=True)
    save_part_of_main_model(
        export_path / "Interfaces.json",
        ["DataTypeDefinitions", "ModuleInterfaces"],
        project_type=ProjectType.INTERFACE,
        skip_validation=skip_validation
    )
    generate_cac_support(export_path, "Interfaces.json", "Interfaces", script_path / "export", ProjectType.INTERFACE)

if __name__ == "__main__":
    export_model()
