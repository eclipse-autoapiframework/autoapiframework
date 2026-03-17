from datetime import timedelta
from vaf import vafpy, BaseTypes

from .imported_models import *

test_module = vafpy.ApplicationModule(name="TestModule", namespace="demo")
test_module.add_provided_interface(
    instance_name="SpeedIfProvider", interface=interfaces.Demo.speed_if
)
test_module.add_provided_interface(
    instance_name="HazardIfProvider", interface=interfaces.Demo.hazard_if
)
test_module.add_consumed_interface(
    instance_name="HazardDetectionIfConsumer",
    interface=interfaces.Demo.hazard_detection_if,
)

periodic_task_200 = vafpy.Task(
    name="PeriodicTask200", period=timedelta(milliseconds=200)
)
test_module.add_task(task=periodic_task_200)

periodic_task_2000 = vafpy.Task(
    name="PeriodicTask2000", period=timedelta(milliseconds=2000)
)
test_module.add_task(task=periodic_task_2000)
