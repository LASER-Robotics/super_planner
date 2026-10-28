cmake_minimum_required(VERSION 3.18)
project(super_planner)

set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Werror=return-type)
add_definitions(-DROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(PCL REQUIRED)
find_package(yaml-cpp REQUIRED)
find_library(DW_LIBRARY dw REQUIRED)

set(ros_libs
  rclcpp
  std_msgs
  sensor_msgs
  tf2_ros
  nav_msgs
  geometry_msgs
  visualization_msgs
  rog_map
  mars_quadrotor_msgs
  pcl_conversions
)
foreach(dependency IN LISTS ros_libs)
  find_package(${dependency} REQUIRED)
endforeach()

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp)
add_library(super STATIC ${srcs})
set_target_properties(super PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(super PUBLIC USE_ROS2 FMT_HEADER_ONLY ORIGIN_AT_CORNER)
target_include_directories(super PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PCL_INCLUDE_DIRS}
)
ament_target_dependencies(super PUBLIC ${ros_libs})
target_link_libraries(super PUBLIC ${PCL_LIBRARIES} yaml-cpp)

add_executable(fsm_node Apps/fsm_node_ros2.cpp)
ament_target_dependencies(fsm_node ament_index_cpp rclcpp)
target_link_libraries(fsm_node super ${DW_LIBRARY})

install(TARGETS super EXPORT export_super_planner
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
install(TARGETS fsm_node RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})

ament_export_targets(export_super_planner HAS_LIBRARY_TARGET)
ament_export_include_directories(include ${PCL_INCLUDE_DIRS})
ament_export_libraries(super)
ament_export_dependencies(${ros_libs} PCL yaml-cpp)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_super_ret_code test/test_super_ret_code.cpp)
  target_include_directories(test_super_ret_code PRIVATE include)
endif()

ament_package()
