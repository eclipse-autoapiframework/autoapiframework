
include(CMakeFindDependencyMacro)

find_dependency(vaf_core)
find_dependency(vaf_data_types)
find_dependency(vaf_data_types)
find_dependency(vaf_module_interfaces)
find_dependency(vaf_test_module_base)
find_dependency(Threads)

include("${CMAKE_CURRENT_LIST_DIR}/test_moduleTargets.cmake")
