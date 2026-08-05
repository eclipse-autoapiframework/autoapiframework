#ifndef SPEED_HAZARD_DETECTION_HPP
#define SPEED_HAZARD_DETECTION_HPP

#include <cstdint>
#include <cstring>

namespace autoapiframework {
namespace speedHazardDetection {

// ============================================================================
// ENUMS - Based on Eclipse-autoapiframework-Metamodel v0.3.0
// ============================================================================

enum class ASIL {
    QM = 0,
    A = 1,
    B = 2,
    C = 3,
    D = 4
};

enum class DataQuality {
    VALID = 0,
    DEFAULT = 1,
    INVALID = 2,
    UNINITIALIZED = 3,
    DEGRADED = 4,
    LIMITED_VALIDITY = 5
};

enum class RunType {
    INIT = 0,
    CYCLIC = 1,
    EVENT = 2,
    TERMINATE = 3
};

enum class Protection {
    NONE = 0,
    COMPLEMENT = 1,
    COMPLEMENT_SEPARATE = 2
};

enum class FunctionResult {
    OK = 0,
    NOT_AVAILABLE = 1,
    INVALID_INPUT = 2,
    COMPLEMENT_ERROR = 3,
    RANGE_ERROR = 4,
    TIMEOUT = 5,
    CALCULATION_ERROR = 6,
    INTERNAL_ERROR = 7
};

// ============================================================================
// DATA INTERFACE STRUCTURES
// ============================================================================

/**
 * @struct VehicleSpeed
 * @brief Current vehicle speed value.
 * 
 * VSS Path: Vehicle.Speed
 * Unit: km/h
 * Range: [0.0, 300.0]
 * ASIL: B
 * Update Period: 10ms
 */
struct VehicleSpeed {
    float value;                    ///< Speed value in km/h
    DataQuality quality;            ///< Data quality indicator
    uint32_t updateTimestampMs;     ///< Last update timestamp in milliseconds
    
    VehicleSpeed() 
        : value(0.0f), quality(DataQuality::UNINITIALIZED), updateTimestampMs(0) {}
    
    VehicleSpeed(float val)
        : value(val), quality(DataQuality::VALID), updateTimestampMs(0) {}
};

/**
 * @struct VehicleAccelerationLongitudinal
 * @brief Longitudinal acceleration used to detect strong acceleration events.
 * 
 * VSS Path: Vehicle.Acceleration.Longitudinal
 * Unit: m/s²
 * Range: [-20.0, 20.0]
 * ASIL: B
 * Update Period: 10ms
 */
struct VehicleAccelerationLongitudinal {
    float value;                    ///< Acceleration value in m/s²
    DataQuality quality;            ///< Data quality indicator
    uint32_t updateTimestampMs;     ///< Last update timestamp in milliseconds
    
    VehicleAccelerationLongitudinal()
        : value(0.0f), quality(DataQuality::UNINITIALIZED), updateTimestampMs(0) {}
    
    VehicleAccelerationLongitudinal(float val)
        : value(val), quality(DataQuality::VALID), updateTimestampMs(0) {}
};

/**
 * @struct VehicleBodyLightsHazardRequest
 * @brief Hazard warning lights request signal.
 * 
 * VSS Path: Vehicle.Body.Lights.Hazard.Request
 * Unit: boolean
 * ASIL: B
 * Update Period: 20ms
 */
struct VehicleBodyLightsHazardRequest {
    bool value;                     ///< Request status: true = activate hazard lights
    DataQuality quality;            ///< Data quality indicator
    uint32_t updateTimestampMs;     ///< Last update timestamp in milliseconds
    
    VehicleBodyLightsHazardRequest()
        : value(false), quality(DataQuality::UNINITIALIZED), updateTimestampMs(0) {}
    
    VehicleBodyLightsHazardRequest(bool val)
        : value(val), quality(DataQuality::VALID), updateTimestampMs(0) {}
};

/**
 * @struct VehicleSpeedQualifier
 * @brief Quality qualifier for Vehicle.Speed value.
 * 
 * VSS Path: Vehicle.Speed.Qualifier
 * Unit: code (enum-based)
 * ASIL: QM
 * Update Period: 10ms
 */
struct VehicleSpeedQualifier {
    uint8_t value;                  ///< Quality code (0-5)
    DataQuality quality;            ///< Data quality indicator
    uint32_t updateTimestampMs;     ///< Last update timestamp in milliseconds
    
    VehicleSpeedQualifier()
        : value(0), quality(DataQuality::UNINITIALIZED), updateTimestampMs(0) {}
    
    VehicleSpeedQualifier(uint8_t val)
        : value(val), quality(DataQuality::VALID), updateTimestampMs(0) {}
};

// ============================================================================
// PARAMETER STRUCTURES
// ============================================================================

/**
 * @struct SpeedHazardDetectionParameters
 * @brief Configuration parameters for SpeedHazardDetection function.
 */
struct SpeedHazardDetectionParameters {
    /// Relative speed increase threshold in percent for forward acceleration
    float speedHazardForwardThresholdPercent;
    
    /// Hold time for hazard request after trigger in milliseconds
    uint32_t hazardRequestDurationMs;
    
    SpeedHazardDetectionParameters()
        : speedHazardForwardThresholdPercent(10.0f),
          hazardRequestDurationMs(3000) {}
};

// ============================================================================
// FUNCTION INPUT/OUTPUT CONTEXT
// ============================================================================

/**
 * @struct SpeedHazardDetectionInputs
 * @brief All input signals for the SpeedHazardDetection function.
 */
struct SpeedHazardDetectionInputs {
    VehicleSpeed vehicleSpeed;
    VehicleAccelerationLongitudinal accelerationLongitudinal;
    VehicleSpeedQualifier speedQualifier;
    
    void reset() {
        vehicleSpeed = VehicleSpeed();
        accelerationLongitudinal = VehicleAccelerationLongitudinal();
        speedQualifier = VehicleSpeedQualifier();
    }
};

/**
 * @struct SpeedHazardDetectionOutputs
 * @brief All output signals for the SpeedHazardDetection function.
 */
struct SpeedHazardDetectionOutputs {
    VehicleBodyLightsHazardRequest hazardRequest;
    FunctionResult functionStatus;
    
    SpeedHazardDetectionOutputs()
        : hazardRequest(), functionStatus(FunctionResult::OK) {}
    
    void reset() {
        hazardRequest = VehicleBodyLightsHazardRequest();
        functionStatus = FunctionResult::OK;
    }
};

// ============================================================================
// FUNCTION CONTEXT / STATE
// ============================================================================

/**
 * @class SpeedHazardDetectionContext
 * @brief Internal state and context for SpeedHazardDetection function.
 * 
 * This structure maintains all internal state variables required for
 * the cyclic execution of the speed hazard detection algorithm.
 */
class SpeedHazardDetectionContext {
public:
    // Initialization state
    bool isInitialized;
    
    // Previous cycle state
    float previousSpeed;
    uint32_t previousSpeedTimestamp;
    
    // Hazard request latching state
    bool hazardRequestActive;
    uint32_t hazardRequestStartTimestamp;
    
    // Cycle counter
    uint32_t cycleCounter;
    
    /**
     * @brief Constructor - initializes context with default values.
     */
    SpeedHazardDetectionContext();
    
    /**
     * @brief Reset context to initial state.
     */
    void reset();
    
    /**
     * @brief Get initialization status.
     * @return true if function is initialized, false otherwise
     */
    bool getInitializationStatus() const;
    
    /**
     * @brief Get current cycle count.
     * @return Current cycle counter value
     */
    uint32_t getCycleCount() const;
};

// ============================================================================
// FUNCTION INTERFACE
// ============================================================================

/**
 * @class SpeedHazardDetection
 * @brief Static C++ API for Speed Hazard Detection Function.
 * 
 * Specification: SpeedHazardDetection v1.0.0
 * Meta Model: Eclipse-autoapiframework-Metamodel v0.3.0
 * 
 * Description:
 *   Detects rapid acceleration and requests hazard warning lights if the
 *   configured threshold is exceeded.
 */
class SpeedHazardDetection {
public:
    /**
     * @brief Initialize the Speed Hazard Detection function.
     * 
     * @param context Reference to the function context
     * @param parameters Reference to the function parameters
     * @return FunctionResult indicating success or error status
     * 
     * Run Type: init
     * ASIL: B
     * 
     * Initializes internal states and latched outputs. Must be called once
     * before the first cyclic execution.
     */
    static FunctionResult init(
        SpeedHazardDetectionContext& context,
        const SpeedHazardDetectionParameters& parameters
    );
    
    /**
     * @brief Execute one cycle of Speed Hazard Detection.
     * 
     * @param context Reference to the function context
     * @param parameters Reference to the function parameters
     * @param inputs Reference to the input signals
     * @param outputs Reference to the output signals (will be updated)
     * @return FunctionResult indicating success or error status
     * 
     * Run Type: cyclic
     * ASIL: B
     * Cycle Time: 20ms
     * 
     * Evaluates inputs and calculates hazard request output.
     * The algorithm:
     *   1. Validates input signal quality
     *   2. Calculates relative speed increase since last cycle
     *   3. Compares against configured threshold
     *   4. Activates hazard request if threshold exceeded
     *   5. Maintains hazard request latch for configured duration
     */
    static FunctionResult step(
        SpeedHazardDetectionContext& context,
        const SpeedHazardDetectionParameters& parameters,
        const SpeedHazardDetectionInputs& inputs,
        SpeedHazardDetectionOutputs& outputs
    );
    
    /**
     * @brief Terminate the Speed Hazard Detection function.
     * 
     * @param context Reference to the function context
     * @param outputs Reference to the output signals (will be updated)
     * @return FunctionResult indicating success or error status
     * 
     * Run Type: terminate
     * ASIL: QM
     * 
     * Shutdown cleanup for deterministic deactivation. Ensures all
     * outputs are reset to safe defaults.
     */
    static FunctionResult terminate(
        SpeedHazardDetectionContext& context,
        SpeedHazardDetectionOutputs& outputs
    );
};

} // namespace speedHazardDetection
} // namespace autoapiframework

#endif // SPEED_HAZARD_DETECTION_HPP
