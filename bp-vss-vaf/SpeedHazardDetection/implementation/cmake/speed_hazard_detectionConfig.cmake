
include(CMakeFindDependencyMacro)

find_dependency(vaf_core)
find_dependency(vaf_data_types)
find_dependency(vaf_data_types)
find_dependency(vaf_module_interfaces)
find_dependency(vaf_speed_hazard_detection_base)
find_dependency(Threads)

include("${CMAKE_CURRENT_LIST_DIR}/speed_hazard_detectionTargets.cmake")
