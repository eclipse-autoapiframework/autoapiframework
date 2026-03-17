from datetime import timedelta

from .application_modules import *
from vaf import *

# If applicable, import the CaC support depending on your middleware for access to platform interfaces and executables.

# --------------------------------------------------------------------------------------------------------------------------
#  Executable definitions
# --------------------------------------------------------------------------------------------------------------------------
demo_executable = Executable("DemoExecutable", timedelta(milliseconds=10))
test_executable = Executable("TestExecutable", timedelta(milliseconds=10))

# --------------------------------------------------------------------------------------------------------------------------
#  Application module instantiations
# --------------------------------------------------------------------------------------------------------------------------
demo_executable.add_application_module(VehicleSpeedCalculation, [])
demo_executable.add_application_module(SpeedHazardDetection, [])

test_executable.add_application_module(TestModule, [])

# --------------------------------------------------------------------------------------------------------------------------
#  Wiring of application and platform modules
# --------------------------------------------------------------------------------------------------------------------------
# TestModule -> VehicleSpeedCalculation via SpeedIf
test_executable.connect_provided_interface_to_silkit(
    TestModule,
    Instances.TestModule.ProvidedInterfaces.SpeedIfProvider,
    "SilkitSpeedIf",
)

demo_executable.connect_consumed_interface_to_silkit(
    VehicleSpeedCalculation,
    Instances.VehicleSpeedCalculation.ConsumedInterfaces.SpeedIfConsumer,
    "SilkitSpeedIf",
)

# VehicleSpeedCalculation -> SpeedHazardDetection via SpeedCalculationIf
demo_executable.connect_interfaces(
    VehicleSpeedCalculation,
    Instances.VehicleSpeedCalculation.ProvidedInterfaces.SpeedCalculationIfProvider,
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ConsumedInterfaces.SpeedCalculationIfConsumer,
)

# TestModule -> SpeedHazardDetection via HazardIf
test_executable.connect_provided_interface_to_silkit(
    TestModule,
    Instances.TestModule.ProvidedInterfaces.HazardIfProvider,
    "SilkitHazardIf",
)

demo_executable.connect_consumed_interface_to_silkit(
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ConsumedInterfaces.HazardIfConsumer,
    "SilkitHazardIf",
)

# SpeedHazardDetection -> TestModule via HazardDetectionIf
demo_executable.connect_provided_interface_to_silkit(
    SpeedHazardDetection,
    Instances.SpeedHazardDetection.ProvidedInterfaces.HazardDetectionIfProvider,
    "SilkitHazardDetectionIf",
)

test_executable.connect_consumed_interface_to_silkit(
    TestModule,
    Instances.TestModule.ConsumedInterfaces.HazardDetectionIfConsumer,
    "SilkitHazardDetectionIf",
)
