%Initialize
Load_Data = 0; %Probably shouldn't have to change this.
Load_GlobalDefines;
Load_Configurables;
Load_SensorConfig;
Load_FilterConfig;
Create_SimulinkBusObjects;
if(exist('Sensor_Data_Loaded') == 0)
    Load_Data = 1;
elseif(Sensor_Data_Loaded == 0)
  Load_Data = 1;
end
disp(['Using Scenario: ' Scenario]);
if(Load_Data == 1)
  disp(['Loading Data...']);
else
  disp(['Data already loaded.']);
end
if(Load_Data == 1)
   if(Truth_Available == 1)
       [Truth_Data_Loaded,truthdata_time_end, ...
           truth_yawrate,truth_pitchrate,truth_rollrate, ...
           truth_forwardaccel,truth_lateralaccel,truth_vertaccel,...
           truth_forwardvel,truth_lateralvel,truth_vertvel,...
           truth_yaw,truth_pitch,truth_roll,...
           truth_east,truth_north,truth_elev] = Load_TruthSignals(Scenario);
   else
       disp(['WARN:] Truth Data Not Available.']);
   end
  [Sensor_Data_Loaded, sensordata_time_end,...
            accel1x_in,accel1y_in,accel1z_in, ...
            accel2x_in,accel2y_in,accel2z_in, ...
            accel3x_in,accel3y_in,accel3z_in, ...
            accel4x_in,accel4y_in,accel4z_in, ...
            rotationrate1x_in,rotationrate1y_in,rotationrate1z_in, ...
            rotationrate2x_in,rotationrate2y_in,rotationrate2z_in, ...
            rotationrate3x_in,rotationrate3y_in,rotationrate3z_in, ...
            rotationrate4x_in,rotationrate4y_in,rotationrate4z_in, ...
            mag1x_in,mag1y_in,mag1z_in, ...
            mag2x_in,mag2y_in,mag2z_in, ...
            mag3x_in,mag3y_in,mag3z_in, ...
            mag4x_in,mag4y_in,mag4z_in, ...
            odom1x_in,odom1y_in,odom1z_in,...
            odom2x_in,odom2y_in,odom2z_in,...
            odom3x_in,odom3y_in,odom3z_in,...
            odom4x_in,odom4y_in,odom4z_in] = Load_SensorSignals(Scenario);
    
    datatime_end = max(truthdata_time_end,sensordata_time_end);
    t = 0:1/ModelConfig.POSE_UPDATE_RATE:datatime_end;
    current_time.time = t;
    current_time.signals.values = t';
    current_time.signals.dimensions = 1;
   
end