/*
 * RobotPlugin.cpp
 *
 *  Created on: Mar 10, 2019
 *      Author: robot
 */

#include "RobotPlugin.h"
using namespace gazebo;
using namespace std;
RobotPlugin::RobotPlugin(): ModelPlugin(),
		m_model(nullptr),
		m_sensorManager(nullptr)
		//m_left_imu(nullptr),
		//m_right_imu(nullptr)
{
	left_cmd = 0.0;
	right_cmd = 0.0;
	// TODO Auto-generated constructor stub
	printf("Plugin opened\n");
}

RobotPlugin::~RobotPlugin() {
	// TODO Auto-generated destructor stub
}

//Initialize Functions
void RobotPlugin::Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
{
	printf("Plugin load started\n");
	m_model = _model;

	InitializePlugin();

	return;

}
bool RobotPlugin::InitializePlugin()
{
	sensors_enabled = true;
	if(LoadModel() == false)
	{
		printf("Model Failed to Load. Exiting.\n");
		return false;
	}
	print_model();
	this->node = transport::NodePtr(new transport::Node());
#if GAZEBO_MAJOR_VERSION < 8
	this->node->Init(m_model->GetWorld()->GetName());
#else
	this->node->Init(this->model->GetWorld()->Name());
#endif
	printf("Starting ros\n");
	if (!ros::isInitialized())
	{
		int argc = 0;
		char **argv = NULL;
		ros::init(argc, argv, "gazebo_client",ros::init_options::NoSigintHandler);
	}
	rosNode.reset(new ros::NodeHandle("gazebo_client"));
	if(InitializeSubscriptions() == false)
	{
		printf("Not able to initialize Subscriptions. Exiting.\n");
		return false;
	}
	if(InitializePublications() == false)
	{
		printf("Not able to initialize Publications. Exiting.\n");
		return false;
	}
	m_fastloop.set_name("FASTLOOP");
	m_fastloop.set_targetrate(50.0);
	m_mediumloop.set_name("MEDIUMLOOP");
	m_mediumloop.set_targetrate(10.0);
	m_slowloop.set_name("SLOWLOOP");
	m_slowloop.set_targetrate(1.0);
	m_veryslowloop.set_name("VERYSLOWLOOP");
	m_veryslowloop.set_targetrate(0.1);
	printf("Plugin Started.\n");
	return true;
}
bool RobotPlugin::LoadModel()
{
	printf("Initializing Model.\n");
	if(LoadSensors() == false)
	{
		printf("Could not load sensors. Exiting.\n");
		return false;
	}
	if(m_model->GetJointCount() == 0)
	{
		printf("Robot Model did not receive any info.  Exiting.\n");
		return false;
	}
	for(uint16_t i = 0; i < m_model->GetJointCount(); ++i)
	{
		if (m_model->GetJoints()[i]->GetScopedName().find("drivetrain") != std::string::npos)
		{
			if (m_model->GetJoints()[i]->GetScopedName().find("left") != std::string::npos)
			{
				joint newjoint;
				newjoint.joint_type = JointType::DRIVETRAIN_LEFT;
				newjoint.id = i;
				newjoint.poweron_setpoint = 0.0;
				newjoint.name = m_model->GetJoints()[i]->GetScopedName();
				joints.push_back(newjoint);
			}
			if (m_model->GetJoints()[i]->GetScopedName().find("right") != std::string::npos)
			{
				joint newjoint;
				newjoint.joint_type = JointType::DRIVETRAIN_RIGHT;
				newjoint.id = i;
				newjoint.poweron_setpoint = 0.0;
				newjoint.name = m_model->GetJoints()[i]->GetScopedName();
				joints.push_back(newjoint);
			}
		}
		if (m_model->GetJoints()[i]->GetScopedName().find("boom_base_joint") != std::string::npos)
		{
			joint newjoint;
			newjoint.joint_type = JointType::BOOM_ROTATE;
			newjoint.id = i;
			newjoint.poweron_setpoint = -1.0;
			newjoint.name = m_model->GetJoints()[i]->GetScopedName();
			joints.push_back(newjoint);
		}
		if (m_model->GetJoints()[i]->GetScopedName().find("bucket_boom_joint") != std::string::npos)
		{
			joint newjoint;
			newjoint.joint_type = JointType::BUCKET_ROTATE;
			newjoint.id = i;
			newjoint.poweron_setpoint = 0.0;
			newjoint.name = m_model->GetJoints()[i]->GetScopedName();
			joints.push_back(newjoint);
		}
	}
	drivetrain_left_pid = common::PID(0.1, 0, 0);
	drivetrain_right_pid = common::PID(0.1, 0, 0);
	boomrotate_pid = common::PID(0.3, 0.01, 0.01);
	bucketrotate_pid = common::PID(0.3, 0.01, 0.01);
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		if(joints.at(i).joint_type == JointType::DRIVETRAIN_LEFT)
		{
			m_model->GetJointController()->SetVelocityPID(
					m_model->GetJoints()[i]->GetScopedName(), drivetrain_left_pid);
			m_model->GetJointController()->SetVelocityTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), joints.at(i).poweron_setpoint);
		}
		if(joints.at(i).joint_type == JointType::DRIVETRAIN_RIGHT)
		{
			m_model->GetJointController()->SetVelocityPID(
					m_model->GetJoints()[i]->GetScopedName(), drivetrain_right_pid);
			m_model->GetJointController()->SetVelocityTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), joints.at(i).poweron_setpoint);
		}
		if(joints.at(i).joint_type == JointType::BOOM_ROTATE)
		{
			m_model->GetJointController()->SetPositionPID(
					m_model->GetJoints()[i]->GetScopedName(), boomrotate_pid);
			m_model->GetJointController()->SetPositionTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), joints.at(i).poweron_setpoint);
		}
		if(joints.at(i).joint_type == JointType::BUCKET_ROTATE)
		{
			m_model->GetJointController()->SetPositionPID(
					m_model->GetJoints()[i]->GetScopedName(), bucketrotate_pid);
			m_model->GetJointController()->SetPositionTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), joints.at(i).poweron_setpoint);
		}

	}
	
	{
		bool status = left_motorcontroller.init("362009");
		if(status == false)
		{
			printf("Could not Initialize Left Motor Controller.\n");
			return false;
		}
	}
	{
		bool status = left_motor.init("361006",45.0);
		if(status == false)
		{
			printf("Could not Initialize Left Motor.\n");
			return false;
		}
	}
	{
		bool status = right_motorcontroller.init("362009");
		if(status == false)
		{
			printf("Could not Initialize Right Motor Controller.\n");
			return false;
		}
	}
	{
		bool status = right_motor.init("361006",45.0);
		if(status == false)
		{
			printf("Could not Initialize Right Motor.\n");
			return false;
		}
	}
	return true;
}

bool RobotPlugin::InitializeSubscriptions()
{
	{
		ros::SubscribeOptions so =
				ros::SubscribeOptions::create<eros::pin>(
						"/LeftMotorController",
						1,
						boost::bind(&RobotPlugin::drivetrain_left_cmd, this, _1),
						ros::VoidPtr(), &this->rosQueue);
		sub_drivetrain_left_cmd = this->rosNode->subscribe(so);
	}
	{
		ros::SubscribeOptions so =
				ros::SubscribeOptions::create<eros::pin>(
						"/RightMotorController",
						1,
						boost::bind(&RobotPlugin::drivetrain_right_cmd, this, _1),
						ros::VoidPtr(), &this->rosQueue);
		sub_drivetrain_right_cmd = this->rosNode->subscribe(so);
	}
	{
		ros::SubscribeOptions so =
				ros::SubscribeOptions::create<eros::pin>(
						"/" +  m_model->GetName() + "/boomrotate_cmd",
						1,
						boost::bind(&RobotPlugin::boom_rotate_cmd, this, _1),
						ros::VoidPtr(), &this->rosQueue);
		sub_boomrotate_cmd = this->rosNode->subscribe(so);
	}
	{
		ros::SubscribeOptions so =
				ros::SubscribeOptions::create<eros::pin>(
						"/" +  m_model->GetName() + "/bucketrotate_cmd",
						1,
						boost::bind(&RobotPlugin::bucket_rotate_cmd, this, _1),
						ros::VoidPtr(), &this->rosQueue);
		sub_bucketrotate_cmd = this->rosNode->subscribe(so);
	}

	this->rosQueueThread =
			std::thread(std::bind(&RobotPlugin::QueueThread, this));

	this->updateConnection = event::Events::ConnectWorldUpdateBegin(
			std::bind(&RobotPlugin::OnUpdate, this));
	return true;
}
bool RobotPlugin::InitializePublications()
{
	if(sensors_enabled == true)
	{
		pub_leftimu = this->rosNode->advertise<eros::imu>("/LeftIMU",1);
		pub_rightimu = this->rosNode->advertise<eros::imu>("/RightIMU",1);
	}
	return true;
}
bool RobotPlugin::LoadSensors()
{
	if(sensors_enabled == true)
	{
	left_imu = initialize_imu("left");
	if(left_imu.initialized == false)
	{
		return false;
	}
	right_imu = initialize_imu("right");
	if(right_imu.initialized == false)
	{
		return false;
	}
	}
	return true;
}
RobotPlugin::IMU RobotPlugin::initialize_imu(std::string location)
{
	IMU m_imu;
	m_imu.initialized = false;
	m_sensorManager = sensors::SensorManager::Instance();
	if(m_sensorManager ==nullptr)
	{
		m_imu.initialized = false;
		return m_imu;
	}
	std::string main_imu_name;
	std::string main_magnetometer_name;
	std::string topic_name;
	if(location == "left")
	{
		main_imu_name = "left_imu";
		main_magnetometer_name = "left_imu_mag";
		topic_name = "/LeftIMU";
	}
	else if(location == "right")
	{
		main_imu_name = "right_imu";
		main_magnetometer_name = "right_imu_mag";
		topic_name = "/RightIMU";
	}
	else
	{
		m_imu.initialized = false;
		return m_imu;
	}
	{
		sensors::SensorPtr _sensor = m_sensorManager->GetSensor(main_imu_name);
		if(_sensor == nullptr)
		{
			printf("ERROR: Could not load %s IMU.  Exiting.\n",main_imu_name.c_str());
			m_imu.initialized = false;
			return m_imu;
		}
		m_imu.m_gazebo_imu = dynamic_pointer_cast<sensors::ImuSensor,sensors::Sensor>(_sensor);
	}
	{
		sensors::SensorPtr _sensor = m_sensorManager->GetSensor(main_magnetometer_name);
		if(_sensor == nullptr)
		{
			printf("ERROR: Could not load %s IMU Magnetometer.  Exiting.\n",main_magnetometer_name.c_str());
			m_imu.initialized = false;
			return m_imu;
		}
		m_imu.m_gazebo_imu_mag = dynamic_pointer_cast<sensors::MagnetometerSensor,sensors::Sensor>(_sensor);
	}
	m_imu.time_imu_updated = 0.0;
	m_imu.time_imumag_updated = 0.0;
	m_imu.eros_imu.sequence_number = 0;
	m_imu.eros_imu.xacc.type = SIGNALTYPE_ACCELERATION;
	m_imu.eros_imu.xacc.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.yacc.type = SIGNALTYPE_ACCELERATION;
	m_imu.eros_imu.yacc.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.zacc.type = SIGNALTYPE_ACCELERATION;
	m_imu.eros_imu.zacc.status = SIGNALSTATE_INITIALIZING;

	m_imu.eros_imu.xgyro.type = SIGNALTYPE_ROTATION_RATE;
	m_imu.eros_imu.xgyro.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.ygyro.type = SIGNALTYPE_ROTATION_RATE;
	m_imu.eros_imu.ygyro.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.zgyro.type = SIGNALTYPE_ROTATION_RATE;
	m_imu.eros_imu.zgyro.status = SIGNALSTATE_INITIALIZING;

	m_imu.eros_imu.xmag.type = SIGNALTYPE_MAGNETIC_FIELD;
	m_imu.eros_imu.xmag.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.ymag.type = SIGNALTYPE_MAGNETIC_FIELD;
	m_imu.eros_imu.ymag.status = SIGNALSTATE_INITIALIZING;
	m_imu.eros_imu.zmag.type = SIGNALTYPE_MAGNETIC_FIELD;
	m_imu.eros_imu.zmag.status = SIGNALSTATE_INITIALIZING;

	m_imu.initialized = true;
	return m_imu;
}
//Update Functions
void RobotPlugin::QueueThread()
{
	static const double timeout = 0.01;
	while (this->rosNode->ok())
	{
		this->rosQueue.callAvailable(ros::WallDuration(timeout));
	}
}
RobotPlugin::IMU RobotPlugin::update_IMU(double current_time,IMU imu)
{
	imu.eros_imu.tov = current_time;
	{
		double time_updated = imu.m_gazebo_imu->LastUpdateTime().Double();
		if(time_updated > imu.time_imu_updated)
		{
			imu.time_imu_updated = time_updated;
			ignition::math::Vector3d acc = imu.m_gazebo_imu->LinearAcceleration();
			ignition::math::Vector3d rate = imu.m_gazebo_imu->AngularVelocity();
			imu.eros_imu.xacc.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.yacc.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.zacc.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.xgyro.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.ygyro.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.zgyro.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.xacc.value = acc.X();
			imu.eros_imu.yacc.value = acc.Y();
			imu.eros_imu.zacc.value = acc.Z();
			imu.eros_imu.xgyro.value = rate.X()*180.0/M_PI;
			imu.eros_imu.ygyro.value = rate.Y()*180.0/M_PI;
			imu.eros_imu.zgyro.value = rate.Z()*180.0/M_PI;
		}
		else
		{
			imu.eros_imu.xacc.status = SIGNALSTATE_HOLD;
			imu.eros_imu.yacc.status = SIGNALSTATE_HOLD;
			imu.eros_imu.zacc.status = SIGNALSTATE_HOLD;
			imu.eros_imu.xgyro.status = SIGNALSTATE_HOLD;
			imu.eros_imu.ygyro.status = SIGNALSTATE_HOLD;
			imu.eros_imu.zgyro.status = SIGNALSTATE_HOLD;
		}	
	}
	{
		double time_updated = imu.m_gazebo_imu_mag->LastUpdateTime().Double();
		if(time_updated > imu.time_imumag_updated)
		{
			ignition::math::Vector3d mag = imu.m_gazebo_imu_mag->MagneticField();
			imu.time_imumag_updated = time_updated;
			imu.eros_imu.xmag.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.ymag.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.zmag.status = SIGNALSTATE_UPDATED;
			imu.eros_imu.xmag.value = mag.X();
			imu.eros_imu.ymag.value = mag.Y();
			imu.eros_imu.zmag.value = mag.Z();
		}
		else
		{
			imu.eros_imu.xmag.status = SIGNALSTATE_HOLD;
			imu.eros_imu.ymag.status = SIGNALSTATE_HOLD;
			imu.eros_imu.zmag.status = SIGNALSTATE_HOLD;
		}
		
	}
	return imu;
	
}
void RobotPlugin::OnUpdate()
{
	if(m_fastloop.run_loop())
	{
		left_motorcontroller.set_batteryvoltage(12.0);
		right_motorcontroller.set_batteryvoltage(12.0);
		if(sensors_enabled == true)
		{
			left_imu = update_IMU(m_fastloop.get_currentTime(),left_imu);
			pub_leftimu.publish(left_imu.eros_imu);
			right_imu = update_IMU(m_fastloop.get_currentTime(),right_imu);
			pub_rightimu.publish(right_imu.eros_imu);
		}
	}
	if(m_mediumloop.run_loop())
	{

	}
	if(m_slowloop.run_loop())
	{
		printf("Left: %4.2f Right: %4.2f\n",left_cmd,right_cmd);
		m_fastloop.check_looprate();
		m_mediumloop.check_looprate();
		m_slowloop.check_looprate();
		m_veryslowloop.check_looprate();
	}
	if(m_veryslowloop.run_loop())
	{
		//print_loopstates(m_veryslowloop);
		//print_loopstates(m_slowloop);
		//print_loopstates(m_mediumloop);
		//print_loopstates(m_fastloop);
	}
}
//Communication Functions
void RobotPlugin::drivetrain_left_cmd(const eros::pin::ConstPtr& _msg)
{
	double left_voltage = left_motorcontroller.set_input(_msg->Value);
	left_cmd = left_motor.set_input(left_voltage);
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		if(joints.at(i).joint_type == JointType::DRIVETRAIN_LEFT)
		{
			m_model->GetJointController()->SetVelocityTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), left_cmd);
		}
	}

}
void RobotPlugin::drivetrain_right_cmd(const eros::pin::ConstPtr& _msg)
{
	double right_voltage = right_motorcontroller.set_input(_msg->Value);
	right_cmd = right_motor.set_input(right_voltage);
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		if(joints.at(i).joint_type == JointType::DRIVETRAIN_RIGHT)
		{
			m_model->GetJointController()->SetVelocityTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), right_cmd);
		}
	}
}
void RobotPlugin::boom_rotate_cmd(const eros::pin::ConstPtr& _msg)
{
	/*
	boomrotate_cmd = _msg->data;
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		if(joints.at(i).joint_type == JointType::BOOM_ROTATE)
		{
			printf("BoomRotate: %f\n",boomrotate_cmd);
			m_model->GetJointController()->SetPositionTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), _msg->data);
		}
	}
	 */
}
void RobotPlugin::bucket_rotate_cmd(const eros::pin::ConstPtr& _msg)
{
	/*
	bucketrotate_cmd = _msg->data;
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		if(joints.at(i).joint_type == JointType::BUCKET_ROTATE)
		{
			printf("BucketRotate: %f\n",bucketrotate_cmd);
			m_model->GetJointController()->SetPositionTarget(
					m_model->GetJoints()[joints.at(i).id]->GetScopedName(), _msg->data);
		}
	}
	 */
}




//Utility Functions
void RobotPlugin::print_loopstates(SimpleTimer timer)
{
	printf("%s: Error: %4.2f%% Target Rate: %4.2f Actual Rate: %4.2f Set Rate: %4.2f\n",timer.get_name().c_str(),fabs(timer.get_timingerrorperc()),
			timer.get_rate(),timer.get_actualrate(),timer.get_setrate());
}
void RobotPlugin::print_model()
{
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		printf("[%d] Type: %s Name: %s\n",
				joints.at(i).id,
				map_jointtype_tostring(joints.at(i).joint_type).c_str(),
				joints.at(i).name.c_str());
	}
}
std::string RobotPlugin::map_jointtype_tostring(uint16_t joint_type)
{
	switch(joint_type)
	{
	case JointType::BOOM_ROTATE:
		return "BOOM ROTATE";
		break;
	case JointType::BUCKET_ROTATE:
		return "BUCKET ROTATE";
		break;
	case JointType::DRIVETRAIN_LEFT:
		return "DRIVETRAIN LEFT";
		break;
	case JointType::DRIVETRAIN_RIGHT:
		return "DRIVETRAIN RIGHT";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}
//Debug
double RobotPlugin::scale_value(double x,double neutral,double x1,double x2,double y1,double y2, double deadband)
{
	double out = 0.0;
	double m = (y2-y1)/(x2-x1);
	//y-y1 = m(x-x1)
	out = m*(x-x1) + y1;
	return out;
}
