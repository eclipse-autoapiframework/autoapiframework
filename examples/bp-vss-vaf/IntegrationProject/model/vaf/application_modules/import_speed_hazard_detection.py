"""Import the application module.

This file is generated. Do not edit manually!
"""

import os
from typing import Optional
from vaf.vafpy import ApplicationModule
from vaf.vafpy.runtime import import_application_module, get_application_module

class ImportedApplicationModule(ApplicationModule):
    def __init__(self, imported_module: Optional[ApplicationModule] = None):
        pass

SpeedHazardDetection: ImportedApplicationModule = ImportedApplicationModule()

model_path = "../../../src/application_modules/speed_hazard_detection/model"

if "IMPORT_APPLICATION_MODULES" in os.environ:
    script_path = os.path.dirname(os.path.realpath(__file__))
    import_application_module(os.path.join(script_path, f"{model_path}/model.json"), "speed_hazard_detection")
    SpeedHazardDetection = get_application_module("SpeedHazardDetection", "demo")
