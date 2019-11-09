%Load_GlobalDefines
eROS_Definitions;

global MODELEXECUTION_MODE;
MODELEXECUTION_MODE.Standard_PoseModel=1;
MODELEXECUTION_MODE.DummyData_TimeCompensate=10;
global RINGBUFFER_SIZE;
RINGBUFFER_SIZE = 500;

global RATES;
RATES.POSE_RATE = 100;

global TIMINGCOMPENSATION_METHOD;
TIMINGCOMPENSATION_METHOD.SampleAndHold = 1;
TIMINGCOMPENSATION_METHOD.LinearExtrapolate = 2;

global OPERATION_MODE;
OPERATION_MODE.CATERPILLAR = 0;
OPERATION_MODE.ICARUS = 1;

global COMMAND_ACTIONS;
COMMAND_ACTIONS.UNDEFINED = 0;
COMMAND_ACTIONS.CHANGEPOSE = 1;
COMMAND_ACTIONS.DRIVE = 2;
COMMAND_ACTIONS.STOP = 3;

global GPS_MODEL;
GPS_MODEL.UNDEFINED = 0;
GPS_MODEL.PERFECT = 1;
GPS_MODEL.STANDARD = 2;
GPS_MODEL.DGPS = 3;
GPS_MODEL.RTK = 4;

global GYROSCOPE_MODEL;
GYROSCOPE_MODEL.UNDEFINED = 0;
GYROSCOPE_MODEL.PERFECT = 1;
GYROSCOPE_MODEL.STANDARD = 2;
GYROSCOPE_MODEL.GOOD = 3;
GYROSCOPE_MODEL.BETTER = 4;

global IMU_MODEL;
IMU_MODEL.UNDEFINED = 0;
IMU_MODEL.PERFECT = 1;

global SIGNAL_STATUS;
SIGNAL_STATUS.UNDEFINED = 0;
SIGNAL_STATUS.INVALID = 1;
SIGNAL_STATUS.INITIALIZING = 2;
SIGNAL_STATUS.UPDATED = 3;
SIGNAL_STATUS.HOLD = 4;
SIGNAL_STATUS.CALIBRATING = 5;

global VARIANCE_BUFFER_SIZE;
VARIANCE_BUFFER_SIZE = 10;

