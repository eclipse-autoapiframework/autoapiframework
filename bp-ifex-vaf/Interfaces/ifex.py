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

import_model(os.path.join(script_path, "ifex-derived-model.json"))

class Vehicle:
    # Enums
    drive_direction = get_datatype("DriveDirection", "Vehicle", "Enum")
    # Type Refs
    speed = get_datatype("Speed", "Vehicle", "TypeRef")
    class Acceleration:
        # Type Refs
        longitudinal = get_datatype("Longitudinal", "Vehicle::Acceleration", "TypeRef")
    class Body:
        class Lights:
            class Hazard:
                # Type Refs
                is_signaling = get_datatype("IsSignaling", "Vehicle::Body::Lights::Hazard", "TypeRef")
                request = get_datatype("Request", "Vehicle::Body::Lights::Hazard", "TypeRef")
    class Chassis:
        # Type Refs
        speed_hazard_forward = get_datatype("SpeedHazardForward", "Vehicle::Chassis", "TypeRef")
        speed_hazard_reverse = get_datatype("SpeedHazardReverse", "Vehicle::Chassis", "TypeRef")
    class Powertrain:
        class Transmission:
            # Enums
            drive_type = get_datatype("DriveType", "Vehicle::Powertrain::Transmission", "Enum")
