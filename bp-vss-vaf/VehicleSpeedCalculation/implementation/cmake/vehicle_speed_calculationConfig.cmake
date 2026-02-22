
include(CMakeFindDependencyMacro)

find_dependency(vaf_core)
find_dependency(vaf_data_types)
find_dependency(vaf_data_types)
find_dependency(vaf_module_interfaces)
find_dependency(vaf_vehicle_speed_calculation_base)
find_dependency(Threads)

include("${CMAKE_CURRENT_LIST_DIR}/vehicle_speed_calculationTargets.cmake")
