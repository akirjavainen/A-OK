# A-OK AC114-01B remote control (RF 433.92MHz) for roller shades and motorized projector screens
A-OK protocol implementation for Arduino and Raspberry Pi. Tested with AM25-1.2/30-MEL-P motors and AC202-02 RF receivers.

https://www.a-okmotors.com/en/

Special thanks to Jason von Nieda for his contribution on reverse engineering the protocol and checksum (https://github.com/akirjavainen/A-OK/issues/1).


# How to use
Capture your remote controls with RemoteCapture.ino and copy paste the 65 bit commands to A-OK.ino for sendAOKCommand(). More info about this provided in RemoteCapture.ino.


# How to use with example commands
1. Set the motor or receiver into pairing mode with its PROGRAM button.
2. Check the instructions on whether you need to send the pairing command ("sendAOKCommand(AOK_PROGRAM_EXAMPLE);") or UP command ("sendAOKCommand(AOK_UP_EXAMPLE);"). Typically you need to transmit within 10 seconds.
3. Now you can control the motor, e.g. "sendAOKCommand(AOK_DOWN_EXAMPLE);" (or AOK_UP_EXAMPLE, AOK_STOP_EXAMPLE etc.).

Setting limits for roller shade motors is quicker with the remotes, although you can use your Arduino for that as well.
