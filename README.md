# A-OK AC114-01B remote control (RF 433.92MHz) for roller shades and motorized projector screens
A-OK protocol implementation for Arduino. Tested with AM25-1.2/30-MEL-P motors and AC202-02 radio receivers.

https://www.a-okmotors.com/en/


# How to use
Capture your remote controls with RemoteCapture.ino and copy paste the 65 bit commands to A-OK.ino for sendAOKCommand(). More info about this provided in RemoteCapture.ino.


# How to use with example commands
1. Set the motor or receiver into pairing mode with its PROGRAM button.
2. Check the instructions on whether you need to send the pairing command ("sendAOKCommand(AOK_PROGRAM_EXAMPLE_1);") or UP command ("sendAOKCommand(AOK_UP_EXAMPLE_1);").
3. Now you can control the motor, eg. sendAOKCommand(AOK_DOWN_1); (or AOK_UP_1, AOK_STOP_1 etc.).

Setting limits is quicker with the remotes, although you can use your Arduino for that as well.
