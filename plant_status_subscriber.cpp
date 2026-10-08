#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// 使用继承 Node 的方式创建节点
class PlantStatusSubscriber : public rclcpp::Node
{
public:
    PlantStatusSubscriber() : Node("plant_status_subscriber")
    {
        //创建订阅者
        // 订阅 /vision/plant_status 话题，使用 std_msgs/msg/String 消息类型
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/vision/plant_status", 10, std::bind(&PlantStatusSubscriber::topic_callback, this, std::placeholders::_1));
    }

private:
    //回调函数
    void topic_callback(const std_msgs::msg::String::SharedPtr msg) const
    {
        // 接收到消息后输出日志
        RCLCPP_INFO(this->get_logger(), "收到视觉检测结果：%s", msg->data.c_str());
    }
    //声明订阅者
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PlantStatusSubscriber>());
    rclcpp::shutdown();
    return 0;
}