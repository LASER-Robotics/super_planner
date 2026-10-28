#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <thread>
#include <unistd.h>

#include <gtest/gtest.h>

static const auto test_directory = std::filesystem::temp_directory_path() /
    ("laser_super_fsm_" + std::to_string(getpid()));
#define LOG_FILE_DIR(name) ((test_directory / (name)).string())
#include <ros_interface/ros2/fsm_ros2.hpp>

class TestFsm : public fsm::FsmRos2 {
public:
    void emergency() { ChangeState("test", EMER_STOP); }
    bool pendingEmergency() const { return machine_state_ == EMER_STOP; }
    void finishedRoute() {
        machine_state_ = FOLLOW_TRAJ;
        finish_plan = true;
        robot_state_.p.setZero();
    }
    bool needsReplan() const { return gi_.new_goal && !finish_plan; }
    Eigen::Vector3d goal() const { return gi_.goal_p; }
};

class SuperFsmTest : public testing::Test {
protected:
    void SetUp() override {
        rclcpp::init(0, nullptr);
        std::filesystem::create_directories(test_directory / "replan_logs");
        std::filesystem::create_directories(test_directory / "cmd_logs");
        const auto config_path = test_directory / "config.yaml";
        auto config = YAML::LoadFile(TEST_CONFIG_PATH);
        config["fsm"]["timer_en"] = false;
        config["astar"]["map_voxel_num"] = std::vector<int>{31, 31, 31};
        auto settings = config["rog_map"];
        settings["map_size"] = std::vector<double>{12, 12, 12};
        settings["resolution"] = 0.25;
        settings["inflation_resolution"] = 0.25;
        settings["frontier_extraction_en"] = false;
        settings["visualization"]["enable"] = false;
        settings["ros_callback"]["enable"] = false;
        std::ofstream(config_path) << config;
        node = std::make_shared<rclcpp::Node>("super_fsm_test");
        fsm = std::make_unique<TestFsm>();
        fsm->init(node, config_path.string());
    }

    void TearDown() override {
        fsm.reset();
        node.reset();
        rclcpp::shutdown();
        std::filesystem::remove_all(test_directory);
    }

    rclcpp::Node::SharedPtr node;
    std::unique_ptr<TestFsm> fsm;
};

TEST_F(SuperFsmTest, MainTimerKeepsEmergencyUntilHeartbeatIsPublished) {
    using Polynomial = mars_quadrotor_msgs::msg::PolynomialTrajectory;
    std::optional<Polynomial> received;
    auto subscription = node->create_subscription<Polynomial>(
        "/planning_cmd/poly_traj", rclcpp::QoS(1).best_effort(),
        [&](const Polynomial &msg) { received = msg; });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (subscription->get_publisher_count() == 0 &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_GT(subscription->get_publisher_count(), 0U);
    fsm->emergency();
    fsm->mainFsmTimerCallback();
    ASSERT_TRUE(fsm->pendingEmergency());
    fsm->pubCmdTimerCallback();
    while (!received && std::chrono::steady_clock::now() < deadline) {
        rclcpp::spin_some(node);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_TRUE(received.has_value());
    EXPECT_NE(received->type & Polynomial::EMER_STOP, 0U);
    EXPECT_FALSE(fsm->pendingEmergency());
}

TEST_F(SuperFsmTest, NewestGoalReplacesAnAlreadyPlannedRoute) {
    fsm->finishedRoute();
    auto goal = std::make_shared<geometry_msgs::msg::PoseStamped>();
    goal->pose.orientation.w = 1.0;
    goal->pose.position.x = 2.0;
    fsm->goalCallback(goal);
    ASSERT_TRUE(fsm->needsReplan());
    EXPECT_NEAR(fsm->goal().x(), 2.0, 0.25);
    goal->pose.position.x = 3.0;
    fsm->goalCallback(goal);
    ASSERT_TRUE(fsm->needsReplan());
    EXPECT_NEAR(fsm->goal().x(), 3.0, 0.25);
}
