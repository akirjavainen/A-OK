/*
******************************************************************************************************************************************************************
*
* A-OK protocol remote control capture
* Compatible with AC114-01B remote
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to serial.
* What you need for mastering your shades are 65 bits commands.
* 
* 
* HOW TO USE
* 
* Plug a 433.92MHz receiver to digital pin 2 and start pressing buttons
* from your original remotes (copy pasting them to A-OK.ino).
*
******************************************************************************************************************************************************************
*/



// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bits capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional info after capture

#define COMMAND_LENGTH  65



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;


  // As a quick hack, we add the leading 1 here.
  // The way I wrote the code misses the first data bit.
  // However, every A-OK protocol command starts with
  // it, so function-wise it's not a big deal:
  String command = "1";


  // *********************************************************************
  // Wait for the first AGC bit:
  // *********************************************************************
  // HIGH between 5230-5310 us
  // *********************************************************************
  
  while (t < 5230 || t > 5310) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)
  }

  if (DEBUG) {
    Serial.print("AGC 1: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for second AGC bit:
  // *********************************************************************
  // LOW between 5040-5150 us
  // *********************************************************************
  
  while (t < 5040 || t > 5150) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for a LOW waveform spike (high-LOW-high)
  }

  if (DEBUG) {
    Serial.print("AGC 2: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 320-370 us
  // 1 = 600-700 us
  // *********************************************************************

  while (i < COMMAND_LENGTH) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 320 && t < 370) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");

    } else if (t > 600 && t < 700) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else { // Unrecognized bit, finish
      if (DEBUG) Serial.println("INVALID BIT");
      i = 0;
      break;
    }

    i++;
  }

  // *********************************************************************
  // Done! Display results:
  // *********************************************************************  

  // Correct data bits length is 65 bits, dismiss bad captures:
  if (command.length() != COMMAND_LENGTH) {
    Serial.print("Bad capture, invalid command length ");
    Serial.println(command.length());
    if (ADDITIONAL) Serial.println("Invalid command: " + command);
    Serial.println();
    
  } else {
    Serial.println("Successful capture, command is: " + command);
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
