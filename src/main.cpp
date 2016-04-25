#include <thread>
#include <chrono>
#include <mutex>
#include <ros/ros.h>
#include <kobuki_msgs/BumperEvent.h>
#include <geometry_msgs/Twist.h>

class Roomba {
public:
	Roomba();

private:
	ros::NodeHandle nh;
	ros::Publisher vel_pub;
	ros::Subscriber bumper_sub;
	std::mutex mu;
	ros::Timer timer;

	void bumperCallback(const kobuki_msgs::BumperEvent msg);
	void applyVel(geometry_msgs::Twist vel);
	void tick();
};

Roomba::Roomba() {
	vel_pub = nh.advertise<geometry_msgs::Twist>("mobile_base/commands/velocity", 1, true);
	bumper_sub = nh.subscribe("mobile_base/events/bumper", 1, &Roomba::bumperCallback, this);
	timer = nh.createTimer(ros::Duration(0.1), std::bind(&Roomba::tick, this));
}

void Roomba::bumperCallback(const kobuki_msgs::BumperEvent msg) {
	if (msg.state == msg.PRESSED) {
		mu.lock();

		geometry_msgs::Twist backup;
		backup.linear.x = -0.15;
		applyVel(backup);

		geometry_msgs::Twist turn;
		turn.angular.z = msg.bumper == msg.LEFT ? -1.0 : 1.0;
		applyVel(turn);

		mu.unlock();
	}
}

void Roomba::applyVel(geometry_msgs::Twist vel) {
	for (int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		vel_pub.publish(vel);
	}
}

void Roomba::tick() {
	mu.lock();
	geometry_msgs::Twist forward;
	forward.linear.x = 0.20;
	vel_pub.publish(forward);
	mu.unlock();
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "roomba");
	Roomba roomba;
	ros::spin();
}
