# LASER SUPER Planner

This repository integrates [HKU-MaRS SUPER](https://github.com/hku-mars/SUPER) into the LASER UAV System (LUS). SUPER plans and replans trajectories around obstacles using a map built from odometry and point-cloud data.

It contains the `super_planner` ROS 2 package and `mars_quadrotor_msgs` interfaces. The exported `super` library also supplies MINCO-S4 to the LUS planning manager for smooth waypoint planning. ROG-Map is a separate dependency provided by [LASER ROG-Map](https://github.com/LASER-Robotics/rog_map).

## Build and run

With this repository, `rog_map` and their dependencies available in a ROS 2 Humble workspace:

```bash
rosdep install --from-paths src --ignore-src -r -y
colcon build --packages-up-to super_planner --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

For LUS, set `planner: super` in the planning-manager YAML and launch `planning_manager`. It starts `fsm_node` and remaps goals, polynomial trajectories, odometry and point clouds. Start `control_manager` separately.

For standalone use:

```bash
ros2 run super_planner fsm_node --ros-args -p config_name:=click_smooth_ros2.yaml -p reference_frame_id:=uav1/odometry -p use_sim_time:=true
```

The standalone command needs the input topics and TF described below. Adapt limits and sensor settings to the vehicle before use.

## Interfaces

The supplied configuration uses these topic names before LUS launch remapping:

| Direction | Topic | Type | Purpose |
| --- | --- | --- | --- |
| Input | `/goal_pose` | `geometry_msgs/msg/PoseStamped` | Navigation goal. |
| Input | `/lidar_slam/odom` | `nav_msgs/msg/Odometry` | Vehicle state for planning and mapping. |
| Input | `/cloud_registered` | `sensor_msgs/msg/PointCloud2` | Point cloud for ROG-Map. |
| Output | `/planning_cmd/poly_traj` | `mars_quadrotor_msgs/msg/PolynomialTrajectory` | Committed position/yaw polynomials and trajectory heartbeat. |
| Output | `/planning/pos_cmd` | `mars_quadrotor_msgs/msg/PositionCommand` | Sampled position command for standalone consumers. |
| Output | `fsm/path` | `nav_msgs/msg/Path` | Vehicle path visualization. |
| Output | `visualization/*` | Markers and point clouds | Planning geometry and debug views. |

The LUS planning manager maps goals to `/<UAV_NAME>/super_planner/goal` and polynomial output to `/<UAV_NAME>/super_planner/trajectory`. It samples the polynomial and publishes `laser_msgs/msg/ReferenceHorizon` for control. Emergency stops are carried in the polynomial heartbeat's `EMER_STOP` flag.

ROG-Map transforms each cloud into the actual odometry frame using TF at the cloud timestamp. A missing transform causes the cloud to be rejected. Use the same clock across sensors, mapping, planning and control. Setting `reference_frame_id` changes the frame label; it does not convert coordinates.

## Configuration

`config_name` selects a YAML file in the installed `super_planner/config/` directory. The supplied [click_smooth_ros2.yaml](super_planner/config/click_smooth_ros2.yaml) configures the node, optimizer and ROG-Map together. It is application configuration, separate from the ROS parameters `config_name`, `reference_frame_id` and `use_sim_time`.

| YAML group | Meaning |
| --- | --- |
| `fsm.click_goal_en`, `click_goal_topic`, `click_height`, `click_yaw_en` | Enables goal input and configures its topic and height/yaw handling. |
| `fsm.replan_rate`, `timer_en` | Replanning rate and timer execution. |
| `fsm.cmd_topic`, `mpc_cmd_topic` | Sampled-command and polynomial-output topics. |
| `super_planner.robot_r` | Robot radius used in planning clearance. |
| `super_planner.planning_horizon`, `sensing_horizon`, `receding_dis` | Planning extent and receding-horizon settings. |
| `super_planner.backup_traj_en` | Enables backup trajectory generation. |
| `super_planner.yaw_mode`, `yaw_dot_max` | Heading behavior and yaw-rate limit. |
| `super_planner.corridor_*`, `safe_corridor_line_max_length`, `iris_iter_num`, `obs_skip_num` | Safe-corridor construction settings. |
| `traj_opt.boundary.*` | Velocity, acceleration, jerk, angular-rate and thrust-acceleration limits. |
| `traj_opt.exp_traj.*`, `traj_opt.backup_traj.*` | Cost weights, convergence and numerical settings for exploration and backup trajectories. |
| `traj_opt.flatness.*` | Vehicle mass, drag, gravity and flatness-model settings. |
| `astar.*` | Search-grid dimensions, heuristic and diagonal-motion settings. |
| `rog_map.resolution`, `inflation_resolution`, `inflation_step`, `map_size` | Map resolution, obstacle inflation and dimensions. |
| `rog_map.ros_callback.*` | Cloud/odometry input topics and odometry timeout. |
| `rog_map.map_sliding.*`, `esdf.*`, `raycasting.*` | Map movement, distance-field and occupancy-update settings. |
| `rog_map.visualization.*` | Map visualization frame, range and update rate. |

The supplied trajectory limits include `max_vel: 10.0` m/s and `max_acc: 20.0` m/s². These are tuning values for the example, not a capability claim for your UAV. Custom YAML topic names must agree with the remappings used by the LUS launch.

## Upstream project and license

SUPER was developed by Yunfan Ren and the MaRS Lab at the University of Hong Kong. Its source files retain their LGPL-3.0-or-later notices; bundled libraries and message definitions retain their own notices and licenses. See the [upstream project](https://github.com/hku-mars/SUPER) for publications and citation information.

The existing integration notes do not establish SITL behavior, flight performance or runtime frequencies under load. Validate those in the intended setup.
