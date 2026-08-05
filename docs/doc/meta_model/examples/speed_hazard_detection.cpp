#include "speed_hazard_detection.hpp"
#include <algorithm>

namespace autoapiframework {
namespace speedHazardDetection {

// ============================================================================
// SpeedHazardDetectionContext Implementation
// ============================================================================

SpeedHazardDetectionContext::SpeedHazardDetectionContext()
    : isInitialized(false),
      previousSpeed(0.0f),
      previousSpeedTimestamp(0),
      hazardRequestActive(false),
      hazardRequestStartTimestamp(0),
      cycleCounter(0) {
}

void SpeedHazardDetectionContext::reset() {
    isInitialized = false;
    previousSpeed = 0.0f;
    previousSpeedTimestamp = 0;
    hazardRequestActive = false;
    hazardRequestStartTimestamp = 0;
    cycleCounter = 0;
}

bool SpeedHazardDetectionContext::getInitializationStatus() const {
    return isInitialized;
}

uint32_t SpeedHazardDetectionContext::getCycleCount() const {
    return cycleCounter;
}

// ============================================================================
// SpeedHazardDetection Function Implementation
// ============================================================================

FunctionResult SpeedHazardDetection::init(
    SpeedHazardDetectionContext& context,
    const SpeedHazardDetectionParameters& parameters) {
    
    // Validate parameters
    if (parameters.speedHazardForwardThresholdPercent < 0.0f) {
        return FunctionResult::INVALID_INPUT;
    }
    
    if (parameters.hazardRequestDurationMs == 0) {
        return FunctionResult::INVALID_INPUT;
    }
    
    // Reset context state
    context.reset();
    
    // Initialize with safe defaults
    context.previousSpeed = 0.0f;
    context.previousSpeedTimestamp = 0;
    context.hazardRequestActive = false;
    context.hazardRequestStartTimestamp = 0;
    context.cycleCounter = 0;
    
    // Mark as initialized
    context.isInitialized = true;
    
    return FunctionResult::OK;
}

FunctionResult SpeedHazardDetection::step(
    SpeedHazardDetectionContext& context,
    const SpeedHazardDetectionParameters& parameters,
    const SpeedHazardDetectionInputs& inputs,
    SpeedHazardDetectionOutputs& outputs) {
    
    // Check initialization
    if (!context.isInitialized) {
        outputs.functionStatus = FunctionResult::NOT_AVAILABLE;
        outputs.hazardRequest.value = false;
        outputs.hazardRequest.quality = DataQuality::INVALID;
        return FunctionResult::NOT_AVAILABLE;
    }
    
    // Increment cycle counter
    context.cycleCounter++;
    
    // ========================================================================
    // INPUT VALIDATION
    // ========================================================================
    
    // Check vehicle speed data quality
    if (inputs.vehicleSpeed.quality == DataQuality::INVALID ||
        inputs.vehicleSpeed.quality == DataQuality::UNINITIALIZED) {
        outputs.functionStatus = FunctionResult::INVALID_INPUT;
        outputs.hazardRequest.quality = DataQuality::INVALID;
        return FunctionResult::INVALID_INPUT;
    }
    
    // Check acceleration data quality
    if (inputs.accelerationLongitudinal.quality == DataQuality::INVALID ||
        inputs.accelerationLongitudinal.quality == DataQuality::UNINITIALIZED) {
        outputs.functionStatus = FunctionResult::INVALID_INPUT;
        outputs.hazardRequest.quality = DataQuality::INVALID;
        return FunctionResult::INVALID_INPUT;
    }
    
    // Validate speed range
    if (inputs.vehicleSpeed.value < 0.0f || inputs.vehicleSpeed.value > 300.0f) {
        outputs.functionStatus = FunctionResult::RANGE_ERROR;
        outputs.hazardRequest.quality = DataQuality::INVALID;
        return FunctionResult::RANGE_ERROR;
    }
    
    // Validate acceleration range
    if (inputs.accelerationLongitudinal.value < -20.0f ||
        inputs.accelerationLongitudinal.value > 20.0f) {
        outputs.functionStatus = FunctionResult::RANGE_ERROR;
        outputs.hazardRequest.quality = DataQuality::INVALID;
        return FunctionResult::RANGE_ERROR;
    }
    
    // ========================================================================
    // SPEED INCREASE DETECTION LOGIC
    // ========================================================================
    
    float currentSpeed = inputs.vehicleSpeed.value;
    float speedIncrease = 0.0f;
    
    if (context.previousSpeedTimestamp > 0) {
        // Calculate time delta in seconds
        uint32_t timeDeltaMs = inputs.vehicleSpeed.updateTimestampMs - 
                               context.previousSpeedTimestamp;
        float timeDeltaS = timeDeltaMs / 1000.0f;
        
        // Ensure we have a valid time delta (at least 1ms)
        if (timeDeltaS > 0.0f) {
            speedIncrease = currentSpeed - context.previousSpeed;
        }
    }
    
    // Calculate relative speed increase in percent
    float relativeSpeedIncreasePercent = 0.0f;
    if (context.previousSpeed > 0.1f) {  // Avoid division by zero
        relativeSpeedIncreasePercent = 
            (speedIncrease / context.previousSpeed) * 100.0f;
    }
    
    // ========================================================================
    // HAZARD REQUEST DECISION LOGIC
    // ========================================================================
    
    bool shouldActivateHazard = false;
    
    // Check if acceleration threshold triggers hazard for forward motion
    if (currentSpeed > 5.0f &&  // Only when vehicle is actively moving
        inputs.accelerationLongitudinal.value > 0.0f &&  // Forward acceleration
        relativeSpeedIncreasePercent >= parameters.speedHazardForwardThresholdPercent) {
        shouldActivateHazard = true;
    }
    
    // ========================================================================
    // HAZARD REQUEST LATCHING
    // ========================================================================
    
    if (shouldActivateHazard) {
        // Activate or re-trigger the hazard request
        context.hazardRequestActive = true;
        context.hazardRequestStartTimestamp = inputs.vehicleSpeed.updateTimestampMs;
    }
    
    // Check if hazard request should remain active based on duration
    if (context.hazardRequestActive) {
        uint32_t hazardDurationElapsedMs = 
            inputs.vehicleSpeed.updateTimestampMs - context.hazardRequestStartTimestamp;
        
        if (hazardDurationElapsedMs >= parameters.hazardRequestDurationMs) {
            // Hazard request duration has expired
            context.hazardRequestActive = false;
        }
    }
    
    // ========================================================================
    // OUTPUT ASSIGNMENT
    // ========================================================================
    
    outputs.hazardRequest.value = context.hazardRequestActive;
    outputs.hazardRequest.quality = DataQuality::VALID;
    outputs.hazardRequest.updateTimestampMs = inputs.vehicleSpeed.updateTimestampMs;
    outputs.functionStatus = FunctionResult::OK;
    
    // ========================================================================
    // STATE UPDATE FOR NEXT CYCLE
    // ========================================================================
    
    context.previousSpeed = currentSpeed;
    context.previousSpeedTimestamp = inputs.vehicleSpeed.updateTimestampMs;
    
    return FunctionResult::OK;
}

FunctionResult SpeedHazardDetection::terminate(
    SpeedHazardDetectionContext& context,
    SpeedHazardDetectionOutputs& outputs) {
    
    // Ensure all outputs are in safe state
    outputs.hazardRequest.value = false;
    outputs.hazardRequest.quality = DataQuality::VALID;
    outputs.hazardRequest.updateTimestampMs = 0;
    outputs.functionStatus = FunctionResult::OK;
    
    // Reset internal state
    context.reset();
    
    return FunctionResult::OK;
}

} // namespace speedHazardDetection
} // namespace autoapiframework
