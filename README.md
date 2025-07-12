# SpeedPulser
Designed to take digital pulses from a speed sensor within the gearbox and translate this into a PWM signal which drives an OEM speedometer.  It’s perfect for situations where a physical speedometer cable cannot be installed or those wishing for a clearer installation.  It is budget-friendly and therefore requires some final user setup and calibration.

The SpeedPulser also accepts speed signals from Can2Cluster – which can get speed from the ECU, DSG or GPS inputs.

The device takes the signals and converts them into signals to suit analog clusters – like the MK1 & MK2.  It is fully opensource and the source code can be downloaded and adjusted to suit other brands if required.  Please get in touch if you need help!

### More Detailed Instructions
The last file on GitHub is the 'Installation Guide' and provides more detail.

## Basic Wiring
Only three wires are required to operate the SpeedPulser:

Inputs:
> 12v ignition 
> Ground
> 12v square wave pulse from gearbox hall sensor or Can2Cluster

Outputs:
> PWM to drive the motor

Software Benefits:
> Fully calibrated motor: Duty Percentage vs. Resulting Speed
> Needle Sweep available
> WiFi Enabled

## WiFi Support
WiFi now enabled for easier on-the-fly setup.  Needle sweep & calibration can be done without needing to mess with complicated firmware.

## Coupler Installation
As the couplers are 3D printed, each coupler, motor housing and speedometer and the alignment of the sun and moon will all be unique and therefore final torque applied to the cluster will be different.  There is a potential for slight variances in final achieved speed.  Therefore, it is the end users requirement to install the coupler and ensure appropriate alignment.  Once installed, the 'highest' speed should be achieved and the potentiometer adjusted to calibrate.  

Press the small button on the right-hand side (closest to the edge of the PCB) to drive the motor at full speed and calibrate to the maximum speed on the speedometer to suit.  This may need a few iterations are parts wear into each other.  

The coupler will be a tight fit on the motor as will the brass driving shaft.  A few light taps will encourge them on.  Remember that this is budget friendly and while a metal alternative would be neat, it's not within budget.

### Trimming the Drive Shaft
Drive shafts are supplied in excess of required length and should be trimmed to suit each individual cluster.  Typically this is ~level with the motor housing but confirmation that it is not bottomed out is key!

## Calibration
If final speedometer reading is incorrect across the range due to tyre size or gearbox variations, the source code can be editted to suit new readings.
The main array for 'Speed' vs. 'PWM' is within '_defs.h' under 'motorPerformance'.  8-bit resolution is duty between 0-100%, but for some fine granularity, 10 bit resolution is the default 'shipped' with.  Therefore there are ~400 entries of PWM, with each having it's own speed.
> uint8_t motorPerformance[] = {0, 0, 0, 0...};

## Understanding the Excel
There is a plot of motor speed vs. PWM in an Excel.  Should it be required, the module can be put into 'testing mode' and each PWM variance worked through in 5 second intervals.  It takes around 10 minutes to carry out a full sweep of the available speeds, but will give good accuracy.  

## Disclaimer
It should be noted that this will drive a speedometer, but it should always be assumed that it is inaccurate and therefore no responsibilty on speed related incidents are at fault of Forbes Automotive.  
