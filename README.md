# LASER SUPER Planner

ROS 2 integration of [HKU-MaRS SUPER](https://github.com/hku-mars/SUPER),
extracted from local integration commit `3731d64` and its validated ROS 2 changes.
Upstream algorithm, author notices and bundled dependency licenses are preserved.

This repository contains `super_planner` and its `mars_quadrotor_msgs` interfaces.
[LASER ROG-Map](https://github.com/LASER-Robotics/rog_map) is a separate dependency.

## Build

From a ROS 2 Humble workspace with the usual ROS build tools installed:

```bash
git clone -b dev/separate_planning_and_control https://github.com/LASER-Robotics/rog_map.git src/rog_map
git clone -b dev/separate_planning_and_control https://github.com/LASER-Robotics/super_planner.git src/super_planner
rosdep install --from-paths src/rog_map src/super_planner --ignore-src -r -y
colcon build --packages-up-to super_planner --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

The exported `super` library includes MINCO-S4 and is also used by the LASER
planning manager. The mapping library is provided by the external `rog_map` package.

## LASER integration

The LASER planning manager launches `fsm_node` only when `planner: super` is selected
in its setup YAML. Launch `control_manager` separately. Planning owns navigation
commands and forwards polynomial trajectories to the control reference pipeline.

`reference_frame_id` names the actual odometry frame. ROG-Map transforms incoming
cloud coordinates into that frame using timestamped TF and rejects missing
transforms. Keep the ROS clock consistent across mapping, planning and control.
Emergency stops propagate through the `EMER_STOP` flag in the trajectory heartbeat.

Standalone node (adjust topics and limits in the selected configuration):

```bash
ros2 run super_planner fsm_node --ros-args -p config_name:=click_smooth_ros2.yaml -p reference_frame_id:=uav1/odometry -p use_sim_time:=true
```

The ROS 2 configuration is loaded from the installed package share directory.
Existing vehicle tuning in `click_smooth_ros2.yaml` is retained; validate its limits
for the UAV before flight. SITL, flight and runtime frequencies under load have not
been validated for this extraction.

SUPER source files retain their upstream LGPL-3.0-or-later notices; bundled libraries
and message definitions retain their own notices and licenses.
