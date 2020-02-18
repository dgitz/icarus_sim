%***************AUTO-GENERATED.  DO NOT EDIT********************/
%***Created on:%2020-02-18 05:43:04.492267%***/
classdef DiagnosticMessage < uint8
	enumeration
		NOERROR_ (0)
		INITIALIZING_ (1)
		INITIALIZING_ERROR_ (2)
		DROPPING_PACKETS_ (4)
		MISSING_HEARTBEATS_ (5)
		DEVICE_NOT_AVAILABLE_ (6)
		ROVER_ARMED_ (7)
		ROVER_DISARMED_ (8)
		TEMPERATURE_HIGH_ (9)
		TEMPERATURE_LOW_ (10)
		DIAGNOSTIC_PASSED_ (11)
		DIAGNOSTIC_FAILED_ (12)
		RESOURCE_LEAK_ (13)
		HIGH_RESOURCE_USAGE_ (14)
		UNKNOWN_STATE_ (15)
		UNKNOWN_MESSAGE_ (255)
	end
end