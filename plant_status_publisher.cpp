#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <chrono>

using namespace std::chrono_literals;

// 使用继承 Node 的方式创建节点
class PlantStatusPublisher : public rclcpp::Node
{
public:
    PlantStatusPublisher() : Node("plant_status_publisher")
    {
        // 使用 std_msgs/msg/String 消息类型，向 /vision/plant_status 话题发布
        //创建发布者
        publisher_ = this->create_publisher<std_msgs::msg::String>("/vision/plant_status", 10);
        
        // 每隔 1 秒发布一次
        timer_ = this->create_wall_timer(
            1s, std::bind(&PlantStatusPublisher::timer_callback, this));
    }

private:
    //编辑回调函数
    void timer_callback()
    {
        auto message = std_msgs::msg::String();
        // 消息的 data 字段固定为：检测到作物
        message.data = "检测到作物";
        
        // 发布消息
        publisher_->publish(message);
        
        // 发布消息后输出日志
        RCLCPP_INFO(this->get_logger(), "发布视觉检测结果：%s", message.data.c_str());
    }
    //声明发布者
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PlantStatusPublisher>());
    rclcpp::shutdown();
    return 0;
}