from datetime import timedelta
from vaf import vafpy, BaseTypes

from .imported_models import *

speed_hazard_detection = vafpy.ApplicationModule(
    name="SpeedHazardDetection", namespace="demo"
)
speed_hazard_detection.add_consumed_interface(
    instance_name="SpeedCalculationIfConsumer",
    interface=interfaces.Demo.speed_calculation_if,
)
speed_hazard_detection.add_consumed_interface(
    instance_name="HazardIfConsumer", interface=interfaces.Demo.hazard_if
)
speed_hazard_detection.add_provided_interface(
    instance_name="HazardDetectionIfProvider",
    interface=interfaces.Demo.hazard_detection_if,
)

periodic_task_10 = vafpy.Task(
    name="HazardCalculation10", period=timedelta(milliseconds=10)
)
speed_hazard_detection.add_task(task=periodic_task_10)
