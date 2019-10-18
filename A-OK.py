#!/usr/bin/python

"""
* A-OK AC114-01B remote control (RF 433.92MHz)
* for roller shades and motorized projector screens
*
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is a Python implementation of the A-OK protocol, for
* the Raspberry Pi. Plug your transmitter to BOARD PIN 16 (BCM/GPIO23).
*
* HOW TO USE
* ./A-OK.py [65-bit_binary_string]
*
* More info on the protocol in A-OK.ino and RemoteCapture.ino here:
* https://github.com/akirjavainen/A-OK
*
"""

import time
import sys
import os
import RPi.GPIO as GPIO


TRANSMIT_PIN = 16  # BCM PIN 23 (GPIO23, BOARD PIN 16)
REPEAT_COMMAND = 8


# Microseconds (us) converted to seconds for time.sleep() function:
AOK_AGC1_PULSE = 0.0053
AOK_AGC2_PULSE = 0.00053
AOK_RADIO_SILENCE = 0.00503

AOK_PULSE_SHORT = 0.00027
AOK_PULSE_LONG = 0.000565

AOK_COMMAND_BIT_ARRAY_SIZE = 65


# ------------------------------------------------------------------
def sendAOKCommand(command):

    if len(str(command)) is not AOK_COMMAND_BIT_ARRAY_SIZE:
        print("Your (invalid) command was", len(str(command)), "bits long.")
        print
        printUsage()

    # Prepare:
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(TRANSMIT_PIN, GPIO.OUT)

    # Send command:
    for t in range(REPEAT_COMMAND):
        doAOKTribitSend(command)

    # Disable output to transmitter and clean up:
    exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def doAOKTribitSend(command):

    # AGC bits:
    transmitHigh(AOK_AGC1_PULSE)  # AGC 1
    transmitLow(AOK_AGC2_PULSE)  # AGC 2

    for i in command:

        if i == '0':  # HIGH-LOW-LOW (100)
            transmitHigh(AOK_PULSE_SHORT)
            transmitLow(AOK_PULSE_LONG)

        elif i == '1':  # HIGH-HIGH-LOW (110)
            transmitHigh(AOK_PULSE_LONG)
            transmitLow(AOK_PULSE_SHORT)

        else:
            print("Invalid character", i, "in command! Exiting...")
            exitProgram()

    # Radio silence:
    transmitLow(AOK_RADIO_SILENCE)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitHigh(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.HIGH)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitLow(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def printUsage():
    print("Usage:")
    print(os.path.basename(sys.argv[0]), "[command_string]")
    print
    print("Correct command length is", AOK_COMMAND_BIT_ARRAY_SIZE, "bits.")
    print
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def exitProgram():
    # Disable output to transmitter and clean up:
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    GPIO.cleanup()
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
# Main program:
# ------------------------------------------------------------------
if len(sys.argv) < 2:
    printUsage()

sendAOKCommand(sys.argv[1])
