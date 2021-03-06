#include "sensorarm_controller_node.h"
bool kill_node = false;
bool SensorArmControllerNode::start(int argc, char **argv)
{
	bool status = false;
	process = new SensorArmControllerProcess();
	set_basenodename(BASE_NODE_NAME);
	initialize_firmware(MAJOR_RELEASE_VERSION, MINOR_RELEASE_VERSION, BUILD_NUMBER, FIRMWARE_DESCRIPTION);
	diagnostic = preinitialize_basenode(argc, argv);
	if (diagnostic.Level > WARN)
	{
		return false;
	}
	

	process->initialize(get_basenodename(), get_nodename(), get_hostname(), DIAGNOSTIC_SYSTEM, DIAGNOSTIC_SUBSYSTEM, DIAGNOSTIC_COMPONENT);
	process->set_config_filepaths("/home/robot/catkin_ws/src/icarus_rover_v2/src_templates/unit_tests/SampleConfig.xml");
	std::vector<uint8_t> diagnostic_types;
	diagnostic_types.push_back(SOFTWARE);
	diagnostic_types.push_back(DATA_STORAGE);
	process->enable_diagnostics(diagnostic_types);
	process->finish_initialization();
	diagnostic = finish_initialization();
	if (diagnostic.Level > WARN)
	{
		return false;
	}
	diagnostic = read_launchparameters();
	if (diagnostic.Level > WARN)
	{
		return false;
	}
	if (diagnostic.Level < WARN)
	{
		diagnostic.Diagnostic_Type = NOERROR;
		diagnostic.Level = INFO;
		diagnostic.Diagnostic_Message = NOERROR;
		diagnostic.Description = "Node Configured.  Initializing.";
		get_logger()->log_diagnostic(diagnostic);
	}
	status = true;
	return status;
}

eros::diagnostic SensorArmControllerNode::read_launchparameters()
{
	eros::diagnostic diag = diagnostic;
	double hold_time;
	std::string param_holdtime = node_name +"/holdtime_step";
	if(n->getParam(param_holdtime,hold_time) == false)
	{
		logger->log_error(__FILE__,__LINE__,"Param: " + param_holdtime + " Not Found. Exiting.");
		diag.Diagnostic_Type = DATA_STORAGE;
		diag.Level = ERROR;
		diag.Diagnostic_Message = INITIALIZING_ERROR;
		return diag;
	}
	process->set_holdsteptime(hold_time);
	for(int i = 0; i < JOINT_COUNT; ++i)
	{
		double start_angle,stop_angle,step_angle;
		std::string param_start_angle = node_name +"/joint" + std::to_string(i) + "_start_angle";
		if(n->getParam(param_start_angle,start_angle) == false)
		{
			logger->log_error(__FILE__,__LINE__,"Param: " + param_start_angle + " Not Found. Exiting.");
			diag.Diagnostic_Type = DATA_STORAGE;
			diag.Level = ERROR;
			diag.Diagnostic_Message = INITIALIZING_ERROR;
			return diag;
		}
		std::string param_stop_angle = node_name +"/joint" + std::to_string(i) + "_stop_angle";
		if(n->getParam(param_stop_angle,stop_angle) == false)
		{
			logger->log_error(__FILE__,__LINE__,"Param: " + param_stop_angle + " Not Found. Exiting.");
			diag.Diagnostic_Type = DATA_STORAGE;
			diag.Level = ERROR;
			diag.Diagnostic_Message = INITIALIZING_ERROR;
			return diag;
		}
		std::string param_step_angle = node_name +"/joint" + std::to_string(i) + "_step_angle";
		if(n->getParam(param_step_angle,step_angle) == false)
		{
			logger->log_error(__FILE__,__LINE__,"Param: " + param_step_angle + " Not Found. Exiting.");
			diag.Diagnostic_Type = DATA_STORAGE;
			diag.Level = ERROR;
			diag.Diagnostic_Message = INITIALIZING_ERROR;
			return diag;
		}
		bool v = process->set_jointproperties(i,start_angle,stop_angle,step_angle);
		if(v == false)
		{
			logger->log_error(__FILE__,__LINE__,"Unable to Set Joint Properties for Joint: " + std::to_string(i));
			diag.Diagnostic_Type = DATA_STORAGE;
			diag.Level = ERROR;
			diag.Diagnostic_Message = INITIALIZING_ERROR;
			return diag;
		}

	}
	get_logger()->log_notice(__FILE__,__LINE__,"Configuration Files Loaded.");
	return diagnostic;
}
eros::diagnostic SensorArmControllerNode::finish_initialization()
{
	eros::diagnostic diag = diagnostic;
	pps1_sub = n->subscribe<std_msgs::Bool>("/1PPS", 1, &SensorArmControllerNode::PPS1_Callback, this);
	command_sub = n->subscribe<eros::command>("/command", 1, &SensorArmControllerNode::Command_Callback, this);
	std::string device_topic = "/" + std::string(host_name) + "_master_node/srv_device";
	srv_device = n->serviceClient<eros::srv_device>(device_topic);
	std::vector<SensorArmControllerProcess::Joint> joints = process->get_joints();
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		ros::Publisher pub = n->advertise<std_msgs::Float64>(joints.at(i).topic_name,1);
		joint_pubs.push_back(pub);
	}
	return diagnostic;
}
bool SensorArmControllerNode::run_001hz()
{
	
	return true;
}
bool SensorArmControllerNode::run_01hz()
{
	return true;
}
bool SensorArmControllerNode::run_01hz_noisy()
{
	std::vector<eros::diagnostic> diaglist = process->get_diagnostics();
	for (std::size_t i = 0; i < diaglist.size(); ++i)
	{
		get_logger()->log_diagnostic(diaglist.at(i));
		diagnostic_pub.publish(diaglist.at(i));
	}
	eros::diagnostic diag = rescan_topics();
	get_logger()->log_diagnostic(diag);
	diagnostic_pub.publish(diag);
	return true;
}
bool SensorArmControllerNode::run_1hz()
{
	if ((process->is_initialized() == true) and (process->is_ready() == true))
	{
	}
	else if ((process->is_ready() == false) and (process->is_initialized() == true))
	{
	}
	else if (process->is_initialized() == false)
	{
		{
			eros::srv_device srv;
			srv.request.query = "SELF";
			if (srv_device.call(srv) == true)
			{
				if (srv.response.data.size() != 1)
				{

					get_logger()->log_error(__FILE__,__LINE__,"Got unexpected device message.");
				}
				else
				{
					new_devicemsg(srv.request.query, srv.response.data.at(0));
				}
			}
			else
			{
			}
		}
	}
	std::vector<eros::diagnostic> diaglist = process->get_diagnostics();
	for (std::size_t i = 0; i < diaglist.size(); ++i)
	{
		if (diaglist.at(i).Level == WARN)
		{
			get_logger()->log_diagnostic(diaglist.at(i));
			diagnostic_pub.publish(diaglist.at(i));
		}
	}
	return true;
}
bool SensorArmControllerNode::run_10hz()
{
	std::vector<SensorArmControllerProcess::Joint> joints = process->get_joints();
	for(std::size_t i = 0; i < joints.size(); ++i)
	{
		std_msgs::Float64 v;
		v.data = joints.at(i).command_position;
		joint_pubs.at(i).publish(v);
	}
	ready_to_arm = process->get_ready_to_arm();
	eros::diagnostic diag = process->update(0.1, ros::Time::now().toSec());
	if (diag.Level > WARN)
	{
		get_logger()->log_diagnostic(diag);
		diagnostic_pub.publish(diag);
	}
	std::vector<eros::diagnostic> diaglist = process->get_diagnostics();
	for (std::size_t i = 0; i < diaglist.size(); ++i)
	{
		if (diaglist.at(i).Level > WARN)
		{
			get_logger()->log_diagnostic(diaglist.at(i));
			diagnostic_pub.publish(diaglist.at(i));
		}
	}
	return true;
}
bool SensorArmControllerNode::run_loop1()
{
	return true;
}
bool SensorArmControllerNode::run_loop2()
{
	return true;
}
bool SensorArmControllerNode::run_loop3()
{
	return true;
}

void SensorArmControllerNode::PPS1_Callback(const std_msgs::Bool::ConstPtr &msg)
{
	new_ppsmsg(msg);
}

void SensorArmControllerNode::Command_Callback(const eros::command::ConstPtr &t_msg)
{
	std::vector<eros::diagnostic> diaglist = process->new_commandmsg(t_msg);
	new_commandmsg_result(t_msg, diaglist);
}
bool SensorArmControllerNode::new_devicemsg(std::string query, eros::device t_device)
{
	if (query == "SELF")
	{
		if (t_device.DeviceName == std::string(host_name))
		{
			set_mydevice(t_device);
			process->set_mydevice(t_device);
		}
	}

	if ((process->is_initialized() == true))
	{
		eros::device::ConstPtr device_ptr(new eros::device(t_device));
		eros::diagnostic diag = process->new_devicemsg(device_ptr);
	}
	return true;
}
eros::diagnostic SensorArmControllerNode::rescan_topics()
{
	eros::diagnostic diag = diagnostic;
	int found_new_topics = 0;

	ros::master::V_TopicInfo master_topics;
	ros::master::getTopics(master_topics);
	for (ros::master::V_TopicInfo::iterator it = master_topics.begin() ; it != master_topics.end(); it++)
	{
		const ros::master::TopicInfo& info = *it;
		if(info.datatype == "eros/command")
		{
			found_new_topics++;
			char tempstr[255];
			sprintf(tempstr,"Subscribing to command topic: %s",info.name.c_str());
			logger->log_info(__FILE__,__LINE__,tempstr);
			ros::Subscriber sub = n->subscribe<eros::command>(info.name,20,&SensorArmControllerNode::Command_Callback,this);
			multiple_subs.push_back(sub);
		}
	}

	char tempstr[255];
	if(found_new_topics > 0)
	{
		sprintf(tempstr,"Rescanned and found %d new topics.",found_new_topics);
	}
	else
	{
		sprintf(tempstr,"Rescanned and found no new topics.");
	}
	diag = process->update_diagnostic(SOFTWARE,INFO,NOERROR,std::string(tempstr));
	logger->log_info(__FILE__,__LINE__,tempstr);
	return diag;
}
void SensorArmControllerNode::thread_loop()
{
	while (kill_node == false)
	{
		ros::Duration(1.0).sleep();
	}
}
void SensorArmControllerNode::cleanup()
{
	base_cleanup();
	get_logger()->log_info(__FILE__,__LINE__,"Node Finished Safely.");
}
/*! \brief Attempts to kill a node when an interrupt is received.
 *
 */
void signalinterrupt_handler(int sig)
{
	printf("Killing Node with Signal: %d", sig);
	kill_node = true;
	exit(0);
}
int main(int argc, char **argv)
{
	signal(SIGINT, signalinterrupt_handler);
	signal(SIGTERM, signalinterrupt_handler);
	SensorArmControllerNode *node = new SensorArmControllerNode();
	bool status = node->start(argc, argv);
	std::thread thread(&SensorArmControllerNode::thread_loop, node);
	while ((status == true) and (kill_node == false))
	{
		status = node->update();
	}
	node->cleanup();
	thread.detach();
	return 0;
}
