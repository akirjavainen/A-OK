# Control A-OK protocol 433.92MHz motorized window shades from Arduino
Tested with AC114-01B remote and AM25-1.2/30-MEL-P motors.

https://www.a-okmotors.com/en/

This is an incomplete protocol implementation for the A-OK window shade motors. I don't personally have any of these motors in use, just happened to stumble upon some units and quickly edited my Markisol protocol code to control them. The motors themselves are of sturdy build and amazingly quiet in operation. If there is anything obviously negative to state about them, it's the short power cable. Since they run on AC power, an extension cord hanging from the top of your window looks kind of ugly (compared to thin DC 5/12/24V cables).


# How to use
Capture your remote controls with RemoteCapture.ino and copy paste the 65 bit commands to A-OK.ino for sendAOKCommand(). More info about this provided in RemoteCapture.ino.


# How to use with example commands
1. Set the shade into pairing mode by holding down its SETTING button until it enters programming mode.
2. Send the pairing command, eg. "sendAOKCommand(AOK_SETTING_1);".
3. Now you can control the shade, eg. sendAOKCommand(AOK_DOWN_1); (or AOK_UP_1, AOK_STOP_1 etc.).

Setting limits is quicker with the remotes, although you can use your Arduino for that as well.
