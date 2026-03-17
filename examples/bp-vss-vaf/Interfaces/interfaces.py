from vaf import BaseTypes, vafpy

from .vss import *

# Definition of Speed Interface
speed_if = vafpy.ModuleInterface(name="SpeedIf", namespace="demo")
speed_if.add_data_element(
    name="Longitudinal", datatype=Vss.Vehicle.Acceleration.longitudinal
)
speed_if.add_data_element(
    name="SpeedRow1WheelLeft", datatype=Vss.Vehicle.Chassis.Axle.Row1.Wheel.Left.speed
)
speed_if.add_data_element(
    name="SpeedRow1WheelRight", datatype=Vss.Vehicle.Chassis.Axle.Row1.Wheel.Right.speed
)
speed_if.add_data_element(
    name="SpeedRow2WheelLeft", datatype=Vss.Vehicle.Chassis.Axle.Row2.Wheel.Left.speed
)
speed_if.add_data_element(
    name="SpeedRow2WheelRight", datatype=Vss.Vehicle.Chassis.Axle.Row2.Wheel.Right.speed
)
speed_if.add_data_element(
    name="DriveType", datatype=Vss.Vehicle.Powertrain.Transmission.drive_type
)

# Definition of Speed Calculation Interface
speed_calculation_if = vafpy.ModuleInterface(
    name="SpeedCalculationIf", namespace="demo"
)
speed_calculation_if.add_data_element(
    name="DriveDirection", datatype=Vss.Vehicle.drive_direction
)
speed_calculation_if.add_data_element(name="Speed", datatype=Vss.Vehicle.speed)

# Definition of Hazard Interface
hazard_if = vafpy.ModuleInterface(name="HazardIf", namespace="demo")
hazard_if.add_data_element(
    name="IsSignaling", datatype=Vss.Vehicle.Body.Lights.Hazard.is_signaling
)
hazard_if.add_data_element(
    name="SpeedHazardForward", datatype=Vss.Vehicle.Chassis.speed_hazard_forward
)
hazard_if.add_data_element(
    name="SpeedHazardReverse", datatype=Vss.Vehicle.Chassis.speed_hazard_reverse
)

# Definition of Hazard Detection Interface
hazard_detection_if = vafpy.ModuleInterface(name="HazardDetectionIf", namespace="demo")
hazard_detection_if.add_data_element(
    name="Request", datatype=Vss.Vehicle.Body.Lights.Hazard.request
)
