"""Import the instances of application modules.

This file is generated. Do not edit manually!
"""
class Instances:

    class TestModule:
    
            class Tasks:
                PeriodicTask200 = "PeriodicTask200"
                PeriodicTask2000 = "PeriodicTask2000"
                
            class ConsumedInterfaces:
                HazardDetectionIfConsumer = "HazardDetectionIfConsumer"
                
            class ProvidedInterfaces:
                SpeedIfProvider = "SpeedIfProvider"
                HazardIfProvider = "HazardIfProvider"
                
    class VehicleSpeedCalculation:
    
            class Tasks:
                SpeedCalculation10 = "SpeedCalculation10"
                DriveDirectionCalculation100 = "DriveDirectionCalculation100"
                
            class ConsumedInterfaces:
                SpeedIfConsumer = "SpeedIfConsumer"
                
            class ProvidedInterfaces:
                SpeedCalculationIfProvider = "SpeedCalculationIfProvider"
                
    class SpeedHazardDetection:
    
            class Tasks:
                HazardCalculation10 = "HazardCalculation10"
                
            class ConsumedInterfaces:
                SpeedCalculationIfConsumer = "SpeedCalculationIfConsumer"
                HazardIfConsumer = "HazardIfConsumer"
                
            class ProvidedInterfaces:
                HazardDetectionIfProvider = "HazardDetectionIfProvider"
                