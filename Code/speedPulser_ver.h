/*
Version History:

todo:
> Add OTA updates
> Add WiFI settings page to adjust 'on the fly'

V1.01 - initial release
V1.02 - added onboard LED pulse to confirm incoming pulses
V1.03 - added Fiat cluster - currently not working <40mph due to 'stickyness' of the cluster and motor not having enough bottom end torque.
      - set to 110mph max (with lower motor voltage), allows full range calibration between 20kmph and 180kmh.  Any more and you're speeding anyway...
V1.04 - added Ford cluster - actually much more linear compared to the VW one!
V1.05 - added 'Global Speed Offset' to allow for motors installed with slight binding.  Will keep the plotted duty/speed curve but offset the WHOLE thing
V1.06 - added 'durationReset' - to reset the motor/duty to 0 after xx ms.  This means when there is a break in pulses (either electrical issue or actually stopped, reset the motor)
V1.07 - added in 160mph clusters for MK2 Golfs (thanks to Charlie for calibration data!)
V1.08 - missed reset dutyCycle to 0 after needle sweep - causes needle to hang slight above zero.
V1.09 - added Martin Springell's MK1 Golf Calibration
V1.10 - added speed multiplier
*/