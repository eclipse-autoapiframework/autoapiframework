import os
from pathlib import Path

from vaf import save_main_model
from vaf.core.common.utils import ProjectType

os.environ["IMPORT_APPLICATION_MODULES"] = "import"

# Import the integration project model
# Change below to switch between the integration project variants.
from . import integration_project
# from . import integration_project_silkit


def export_model(skip_validation: bool = False):
    script_path = Path(__file__).resolve().parent
    save_main_model(
        script_path / "model.json",
        project_type=ProjectType.INTEGRATION,
        skip_validation=skip_validation,
    )


if __name__ == "__main__":
    export_model()
