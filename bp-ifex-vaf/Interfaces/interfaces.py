from vaf import BaseTypes, vafpy

from .ifex import *

# Definition of Speed Interface
speed_if = vafpy.ModuleInterface(name="SpeedIf", namespace="demo")
speed_if.add_data_element(
    name="Longitudinal", datatype=Vehicle.Acceleration.longitudinal
)
speed_if.add_data_element(name="SpeedRow1WheelLeft", datatype=Vehicle.speed)
speed_if.add_data_element(name="SpeedRow1WheelRight", datatype=Vehicle.speed)
speed_if.add_data_element(name="SpeedRow2WheelLeft", datatype=Vehicle.speed)
speed_if.add_data_element(name="SpeedRow2WheelRight", datatype=Vehicle.speed)
speed_if.add_data_element(
    name="DriveType", datatype=Vehicle.Powertrain.Transmission.drive_type
)

# Definition of Speed Calculation Interface
speed_calculation_if = vafpy.ModuleInterface(
    name="SpeedCalculationIf", namespace="demo"
)
speed_calculation_if.add_data_element(
    name="DriveDirection", datatype=Vehicle.drive_direction
)
speed_calculation_if.add_data_element(name="Speed", datatype=Vehicle.speed)

# Definition of Hazard Interface
hazard_if = vafpy.ModuleInterface(name="HazardIf", namespace="demo")
hazard_if.add_data_element(
    name="IsSignaling", datatype=Vehicle.Body.Lights.Hazard.is_signaling
)
hazard_if.add_data_element(
    name="SpeedHazardForward", datatype=Vehicle.Chassis.speed_hazard_forward
)
hazard_if.add_data_element(
    name="SpeedHazardReverse", datatype=Vehicle.Chassis.speed_hazard_reverse
)

# Definition of Hazard Detection Interface
hazard_detection_if = vafpy.ModuleInterface(name="HazardDetectionIf", namespace="demo")
hazard_detection_if.add_data_element(
    name="Request", datatype=Vehicle.Body.Lights.Hazard.request
)
