/*
 * RobotPlugin.h
 *
 *  Created on: Mar 10, 2019
 *      Author: robot
 */

#ifndef SRC_ICARUS_SIM_SRC_ROBOTPLUGIN_H_
#define SRC_ICARUS_SIM_SRC_ROBOTPLUGIN_H_

//C System Files

//C++ System Files
#include <thread>
#include <vector>
#include <stdint.h>
#include "string"
//Gazebo Base Functionality
#include <gazebo/gazebo.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/physics/physics.hh>
#include <functional>
#include <gazebo/sensors/SensorManager.hh>
#include <gazebo/sensors/ImuSensor.hh>
#include <gazebo/sensors/MagnetometerSensor.hh>
#include <ignition/math/Vector3.hh>
//ROS Base Functionality
#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "ros/subscribe_options.h"
//Gazebo Messages
#include <gazebo/msgs/msgs.hh>
//ROS Messages
#include "std_msgs/Float32.h"
#include <eros/signal.h>
#include <eros/imu.h>
//Project
#include "../include/SimpleTimer.h"
#include "../../eROS/include/eROS_Definitions.h"

namespace gazebo
{
/*! \class RobotPlugin RobotPlugin.h "RobotPlugin.h"
 *  \brief This is a RobotPlugin class.  This plugin can be used by Gazebo.*/
class RobotPlugin: public ModelPlugin {
public:
	RobotPlugin();
	virtual ~RobotPlugin();
	//Constants
	//Structs
	//Initialization Functions
	virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);
	//Update Functions
	virtual void OnUpdate();
	//Utility Functions
	void print_model();
	//Message Functions
	void drivetrain_left_cmd(const std_msgs::Float32ConstPtr &_msg);
	void drivetrain_right_cmd(const std_msgs::Float32ConstPtr &_msg);
	void boom_rotate_cmd(const std_msgs::Float32ConstPtr &_msg);
	void bucket_rotate_cmd(const std_msgs::Float32ConstPtr &_msg);
	//Destructors

private:
	//Initialize Functions
	eros::imu initialize_imu();
	bool InitializePlugin();
	bool LoadModel();
	bool LoadSensors();
	bool InitializeSubscriptions();
	bool InitializePublications();
	//Update Functions
	void QueueThread();
	//Utility Functions
	std::string map_jointtype_tostring(uint16_t joint_type);
	void print_loopstates(SimpleTimer timer);
	//Structs
	enum JointType
	{
		UNKNOWN = 0,
		DRIVETRAIN_LEFT =1,
		DRIVETRAIN_RIGHT = 2,
		BOOM_ROTATE = 3,
		BUCKET_ROTATE = 4,

	};
	struct joint
	{
		JointType joint_type;
		double poweron_setpoint;
		uint16_t id;
		std::string name;
	};

	//Communication Variables
	std::unique_ptr<ros::NodeHandle> rosNode;
	transport::NodePtr node;
	event::ConnectionPtr updateConnection;
	ros::CallbackQueue rosQueue;
	physics::ModelPtr m_model;
	std::thread rosQueueThread;

	ros::Subscriber sub_drivetrain_left_cmd;
	ros::Subscriber sub_drivetrain_right_cmd;
	ros::Subscriber sub_boomrotate_cmd;
	ros::Subscriber sub_bucketrotate_cmd;
	ros::Publisher pub_leftimu;
	ros::Publisher pub_rightimu;

	//Timing Variables
	SimpleTimer m_fastloop;
	SimpleTimer m_mediumloop;
	SimpleTimer m_slowloop;
	SimpleTimer m_veryslowloop;

	//State Variables
	std::vector<joint> joints;
	common::PID drivetrain_left_pid;
	common::PID drivetrain_right_pid;
	double left_cmd;
	double right_cmd;
	common::PID boomrotate_pid;
	common::PID bucketrotate_pid;
	double boomrotate_cmd;
	double bucketrotate_cmd;

	//Sensor Variables
	sensors::SensorManager *m_sensorManager;
	sensors::ImuSensorPtr m_left_imu;
	sensors::ImuSensorPtr m_right_imu;
	sensors::MagnetometerSensorPtr m_left_imu_mag;
	sensors::MagnetometerSensorPtr m_right_imu_mag;
	eros::imu left_imu;
	eros::imu right_imu;

















};
GZ_REGISTER_MODEL_PLUGIN(RobotPlugin)
}
#endif /* SRC_ICARUS_SIM_SRC_ROBOTPLUGIN_H_ */
