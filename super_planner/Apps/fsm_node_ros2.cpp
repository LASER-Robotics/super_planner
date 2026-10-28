/**
 * This file is part of SUPER
 *
 * Copyright 2025 Yunfan REN, MaRS Lab, University of Hong Kong, <mars.hku.hk>
 * Developed by Yunfan REN <renyf at connect dot hku dot hk>
 * for more information see <https://github.com/hku-mars/SUPER>.
 * If you use this code, please cite the respective publications as
 * listed on the above website.
 *
 * SUPER is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SUPER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SUPER. If not, see <http://www.gnu.org/licenses/>.
 */

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <filesystem>
#include <ros_interface/ros2/fsm_ros2.hpp>

#define BACKWARD_HAS_DW 1
#include "utils/header/backward.hpp"

namespace backward
{
SignalHandling sh;
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  pcl::console::setVerbosityLevel(pcl::console::L_ALWAYS);
  auto node = std::make_shared<rclcpp::Node>("fsm_node");
  std::error_code log_error;
  std::filesystem::create_directories(std::string(ROOT_DIR) + "log/replan_logs", log_error);
  if (log_error) {
    RCLCPP_WARN(node->get_logger(), "Cannot create planner log directory: %s",
                log_error.message().c_str());
  }
  const auto config_name = node->declare_parameter<std::string>(
    "config_name", "click_smooth_ros2.yaml");
  const auto config_path = ament_index_cpp::get_package_share_directory("super_planner") +
    "/config/" + config_name;
  auto fsm = std::make_shared<fsm::FsmRos2>();
  fsm->init(node, config_path);
  RCLCPP_INFO(node->get_logger(), "SUPER initialized with %s", config_path.c_str());

  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
