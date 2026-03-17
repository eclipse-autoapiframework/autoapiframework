import os

from vaf.vafpy import Executable
from vaf.vafpy.runtime import (
    get_datatype,
    get_module_interface,
    get_platform_consumer_module,
    get_platform_provider_module,
    import_model,
)

script_path = os.path.dirname(os.path.realpath(__file__))

import_model(os.path.join(script_path, "Interfaces.json"))

class Demo:
    # Module Interfaces
    speed_if = get_module_interface(
        "SpeedIf", "demo"
    )
    speed_calculation_if = get_module_interface(
        "SpeedCalculationIf", "demo"
    )
    hazard_if = get_module_interface(
        "HazardIf", "demo"
    )
    hazard_detection_if = get_module_interface(
        "HazardDetectionIf", "demo"
    )
class Vss:
    class Vehicle:
        # Enums
        drive_direction = get_datatype("DriveDirection", "vss::vehicle", "Enum")
        # Type Refs
        speed = get_datatype("Speed", "vss::vehicle", "TypeRef")
        class Acceleration:
            # Type Refs
            longitudinal = get_datatype("Longitudinal", "vss::vehicle::acceleration", "TypeRef")
        class Body:
            class Lights:
                class Hazard:
                    # Type Refs
                    is_signaling = get_datatype("IsSignaling", "vss::vehicle::body::lights::hazard", "TypeRef")
                    request = get_datatype("Request", "vss::vehicle::body::lights::hazard", "TypeRef")
        class Chassis:
            # Type Refs
            speed_hazard_forward = get_datatype("SpeedHazardForward", "vss::vehicle::chassis", "TypeRef")
            speed_hazard_reverse = get_datatype("SpeedHazardReverse", "vss::vehicle::chassis", "TypeRef")
        class Powertrain:
            class Transmission:
                # Enums
                drive_type = get_datatype("DriveType", "vss::vehicle::powertrain::transmission", "Enum")
