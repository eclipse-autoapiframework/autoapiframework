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

import_model(os.path.join(script_path, "vss-derived-model.json"))

class Vaf:
    # Strings
    string = get_datatype("String", "vaf", "String")
class Vss:
    # Structs
    vehicle = get_datatype("Vehicle", "vss", "Struct")
    class Vehicle:
        # Enums
        drive_direction = get_datatype("DriveDirection", "vss::vehicle", "Enum")
        # Structs
        acceleration = get_datatype("Acceleration", "vss::vehicle", "Struct")
        powertrain = get_datatype("Powertrain", "vss::vehicle", "Struct")
        chassis = get_datatype("Chassis", "vss::vehicle", "Struct")
        body = get_datatype("Body", "vss::vehicle", "Struct")
        # Type Refs
        speed = get_datatype("Speed", "vss::vehicle", "TypeRef")
        class Acceleration:
            # Type Refs
            longitudinal = get_datatype("Longitudinal", "vss::vehicle::acceleration", "TypeRef")
        class Body:
            # Structs
            lights = get_datatype("Lights", "vss::vehicle::body", "Struct")
            class Lights:
                # Structs
                hazard = get_datatype("Hazard", "vss::vehicle::body::lights", "Struct")
                class Hazard:
                    # Type Refs
                    is_signaling = get_datatype("IsSignaling", "vss::vehicle::body::lights::hazard", "TypeRef")
                    request = get_datatype("Request", "vss::vehicle::body::lights::hazard", "TypeRef")
        class Chassis:
            # Structs
            axle = get_datatype("Axle", "vss::vehicle::chassis", "Struct")
            # Type Refs
            speed_hazard_reverse = get_datatype("SpeedHazardReverse", "vss::vehicle::chassis", "TypeRef")
            speed_hazard_forward = get_datatype("SpeedHazardForward", "vss::vehicle::chassis", "TypeRef")
            class Axle:
                # Structs
                row1 = get_datatype("Row1", "vss::vehicle::chassis::axle", "Struct")
                row2 = get_datatype("Row2", "vss::vehicle::chassis::axle", "Struct")
                class Row1:
                    # Structs
                    wheel = get_datatype("Wheel", "vss::vehicle::chassis::axle::row1", "Struct")
                    class Wheel:
                        # Structs
                        right = get_datatype("Right", "vss::vehicle::chassis::axle::row1::wheel", "Struct")
                        left = get_datatype("Left", "vss::vehicle::chassis::axle::row1::wheel", "Struct")
                        class Left:
                            # Type Refs
                            speed = get_datatype("Speed", "vss::vehicle::chassis::axle::row1::wheel::left", "TypeRef")
                        class Right:
                            # Type Refs
                            speed = get_datatype("Speed", "vss::vehicle::chassis::axle::row1::wheel::right", "TypeRef")
                class Row2:
                    # Structs
                    wheel = get_datatype("Wheel", "vss::vehicle::chassis::axle::row2", "Struct")
                    class Wheel:
                        # Structs
                        left = get_datatype("Left", "vss::vehicle::chassis::axle::row2::wheel", "Struct")
                        right = get_datatype("Right", "vss::vehicle::chassis::axle::row2::wheel", "Struct")
                        class Left:
                            # Type Refs
                            speed = get_datatype("Speed", "vss::vehicle::chassis::axle::row2::wheel::left", "TypeRef")
                        class Right:
                            # Type Refs
                            speed = get_datatype("Speed", "vss::vehicle::chassis::axle::row2::wheel::right", "TypeRef")
        class Powertrain:
            # Structs
            transmission = get_datatype("Transmission", "vss::vehicle::powertrain", "Struct")
            class Transmission:
                # Enums
                drive_type = get_datatype("DriveType", "vss::vehicle::powertrain::transmission", "Enum")
