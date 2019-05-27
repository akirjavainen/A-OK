/*
******************************************************************************************************************************************************************
*
* A-OK protocol remote control capture
* Compatible with AC114-01B remote
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to
* serial (Tools -> Serial Monitor). What you need for mastering your
* motors are 65 bits commands.
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
#define RECEIVE_PIN       2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bits capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG             false
#define ADDITIONAL        false    // Display some additional info after capture

#define COMMAND_LENGTH    65


String last_command = "";


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
  String command = "";


  // *********************************************************************
  // Wait for the AGC bit
  // *********************************************************************
  // NOTE: Some AC114-01B remotes precede this with a preamble
  // (repeated 8 times):
  // *********************************************************************  
  // HIGH of approx. 340 us
  // LOW of approx. 520 us
  // *********************************************************************
  // But we'll ignore this preamble since not all remotes transmit it.
  // All remotes start the command with an AGC of HIGH between 4800-5500 us
  // *********************************************************************
  
  while (t < 4800 || t > 5500) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it
  }

  if (DEBUG) {
    Serial.print("AGC: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 25-370 us
  // 1 = 500-800 us
  // *********************************************************************

  while (i < COMMAND_LENGTH) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 25 && t < 370) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");

    } else if (t > 500 && t < 800) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else { // Unrecognized bit (could be the next command's starting AGC), finish
      if (DEBUG) {
        Serial.print("INVALID BIT");
        Serial.println(t);
      }

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
    
    if (ADDITIONAL) {
      Serial.println("Invalid command: " + command);
    }
    
  } else {
    if (command != last_command) { // Only print unique commands to serial for easier readability
      last_command = command;
      Serial.print("Successful capture, command is: ");
      Serial.println(command);
      
    } else {
      Serial.println("Command repeated");
    }
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
