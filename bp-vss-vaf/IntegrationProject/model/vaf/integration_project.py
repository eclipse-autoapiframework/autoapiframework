from datetime import timedelta

from .application_modules import SpeedHazardDetection, TestModule, Instances, VehicleSpeedCalculation
from vaf import *

# If applicable, import the CaC support depending on your middleware for access to platform interfaces and executables.

# --------------------------------------------------------------------------------------------------------------------------
#  Executable definitions
# --------------------------------------------------------------------------------------------------------------------------
executable = Executable("DemoExecutable", timedelta(milliseconds=10))

# --------------------------------------------------------------------------------------------------------------------------
#  Application module instantiations
# --------------------------------------------------------------------------------------------------------------------------
executable.add_application_module(VehicleSpeedCalculation, [])
executable.add_application_module(SpeedHazardDetection, [])

executable.add_application_module(TestModule, [])

# --------------------------------------------------------------------------------------------------------------------------
#  Wiring of application modules
# --------------------------------------------------------------------------------------------------------------------------
# TestModule -> VehicleSpeedCalculation via SpeedIf
executable.connect_interfaces(
    TestModule,
    Instances.TestModule.ProvidedInterfaces.SpeedIfProvider,
    VehicleSpeedCalculation,
    Instances.VehicleSpeedCalculation.ConsumedInterfaces.SpeedIfConsumer,
)

# VehicleSpeedCalculation -> SpeedHazardDetection via SpeedCalculationIf
executable.connect_interfaces(
    VehicleSpeedCalculation,
    Instances.VehicleSpeedCalculation.ProvidedInterfaces.SpeedCalculationIfProvider,
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ConsumedInterfaces.SpeedCalculationIfConsumer,
)

# TestModule -> SpeedHazardDetection via HazardIf
executable.connect_interfaces(
    TestModule,
    Instances.TestModule.ProvidedInterfaces.HazardIfProvider,
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ConsumedInterfaces.HazardIfConsumer,
)

# SpeedHazardDetection -> TestModule via HazardDetectionIf
executable.connect_interfaces(
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ProvidedInterfaces.HazardDetectionIfProvider,
    TestModule,
    Instances.TestModule.ConsumedInterfaces.HazardDetectionIfConsumer,
)
