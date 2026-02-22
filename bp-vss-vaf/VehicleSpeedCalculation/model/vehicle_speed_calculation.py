from datetime import timedelta
from vaf import vafpy, BaseTypes

from .imported_models import *

vehicle_speed_calculation = vafpy.ApplicationModule(
    name="VehicleSpeedCalculation", namespace="demo"
)
vehicle_speed_calculation.add_consumed_interface(
    instance_name="SpeedIfConsumer", interface=interfaces.Demo.speed_if
)
vehicle_speed_calculation.add_provided_interface(
    instance_name="SpeedCalculationIfProvider",
    interface=interfaces.Demo.speed_calculation_if,
)

periodic_task_10 = vafpy.Task(
    name="SpeedCalculation10", period=timedelta(milliseconds=10)
)
vehicle_speed_calculation.add_task(task=periodic_task_10)

periodic_task_100 = vafpy.Task(
    name="DriveDirectionCalculation100", period=timedelta(milliseconds=100)
)
vehicle_speed_calculation.add_task(task=periodic_task_100)
