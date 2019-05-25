/*
******************************************************************************************************************************************************************
*
* A-OK AC114-01B remote control (RF 433.92MHz) for window shades and projector screens
* Control code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* https://www.a-okmotors.com/en/
* 
* This is an incomplete implementation of the A-OK protocol. I only have one remote control.
* 
* 
* HOW TO USE
* 
* Capture your remote controls with RemoteCapture.ino and copy paste the 65 bit
* commands to A-OK.ino for sendAOKCommand(). More info about this
* provided in RemoteCapture.ino.
* 
* 
* HOW TO USE WITH EXAMPLE COMMANDS
* 
* 1. Set the shade into pairing mode by holding down its PROGRAM button until it enters programming mode.
* 2. Send the pairing command, eg. "sendAOKCommand(AOK_PROGRAM_EXAMPLE);".
* 3. Now you can control the shade, eg. sendAOKCommand(AOK_DOWN_EXAMPLE); (or AOK_UP_EXAMPLE, AOK_STOP_EXAMPLE etc.).
* 
* Setting limits is quicker with the remotes, although you can use your Arduino for that as well.
*
*
* PROTOCOL DESCRIPTION
* 
* One remote control is not enough data to understand the protocol, but here's what I've
* figured out so far.
* 
* UP and DOWN buttons send two different commands for some reason, listed below
* as AOK_UP/DOWN_EXAMPLE and AOK_AFTER_UP_DOWN_EXAMPLE. However, the latter command
* would not seem to be required at all.
* 
* Tri-state bits are used.
* 
* AGC:
* Some remotes start the first command with 8 times: HIGH of approx. 340 us + LOW of approx. 520 us
* But most remotes do not transmit that preamble.
* Every remote starts the command with an AGC HIGH of approx. 5200 us.
* Then go low for approx. 530 us and start the commands bits.
* 
* COMMAND BITS:
* 49 bits for a unique remote ID
* 7 bits for command (UP = 0001011, DOWN = 1000011, STOP = 0100011, PROGRAM = 1010011, AFTER UP/DOWN = 0100100)
* 5 bits are unknown (maybe some kind of checksum?)
* 4 bits at the end are always 0111, except for the AFTER UP/DOWN commands 1001
* 
* = a total of 65 command bits
* 
* RADIO SILENCE:
* Some remotes instantly repeat the commands, some transmit a radio silence of approx. 5030 us at the end
* of each command.
* 
* 
* TIMINGS:
* All sample counts below listed with a sample rate of 44100 Hz
* (sample count / 44100 = microseconds).
* 
* Pulse length:
* SHORT (LOW): approx. 7 samples = 159 us
* MEDIUM (HIGH-HIGH): approx. 18 samples = 408 us
* LONG (HIGH-HIGH-HIGH): approx. 32 samples = 726 us
*
* Data bits:
* Data 0 = MEDIUM LOW - MEDIUM HIGH - SHORT LOW (wire 00110)
* Data 1 = LONG HIGH - SHORT LOW (wire 1110)
* 
* End with LOW radio silence of (minimum) 11 samples = 249 us
*
******************************************************************************************************************************************************************
*/



// Example commands:
#define AOK_DOWN_EXAMPLE              "10100011010001100101000000010110000000010000000001000011111100001"
#define AOK_UP_EXAMPLE                "10100011010001100101000000010110000000010000000000001011101110001"
#define AOK_AFTER_UP_DOWN_EXAMPLE     "10100011010001100101000000010110000000010000000000100100110100011"
#define AOK_STOP_EXAMPLE              "10100011010001100101000000010110000000010000000000100011110100001"
#define AOK_PROGRAM_EXAMPLE           "10100011010001100101000000010110000000010000000001010011000000001"


#define TRANSMIT_PIN             13      // We'll use digital 13 for transmitting
#define REPEAT_COMMAND           8       // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG                    false   // Do note that if you add serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define AOK_AGC1_PULSE                   5300  // 234 samples
#define AOK_AGC2_PULSE                   530   // 24 samples after the actual AGC bit
#define AOK_RADIO_SILENCE                5030  // 222 samples

#define AOK_PULSE_SHORT                  270   // 12 samples
#define AOK_PULSE_LONG                   565   // 25 samples, approx. 2 * AOK_PULSE_SHORT

#define AOK_COMMAND_BIT_ARRAY_SIZE       65    // Command bit count



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(9600); // Used for error messages even with DEBUG set to false
      
  if (DEBUG) Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {

  // Pair a shade (first set the shade to pairing mode by holding
  // down its setting key), then send the pairing command:
  //sendAOKCommand(AOK_PROGRAM_EXAMPLE);
  //while (true) {} // Stop after pairing, you can use UP/STOP/DOWN commands afterwards
  // ---

  // Send the command:
  sendAOKCommand(AOK_UP_EXAMPLE);
  delay(5000);
  sendAOKCommand(AOK_STOP_EXAMPLE);
  delay(5000);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendAOKCommand(String command) {
  
  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  if (command.length() < AOK_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendAOKCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (command.length() > AOK_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendAOKCommand(): Invalid command (too long), cannot continue.");
    return;
  }

  // Declare the array (int) of command bits
  int command_array[AOK_COMMAND_BIT_ARRAY_SIZE];

  // Processing a string during transmit is just too slow,
  // let's convert it to an array of int first:
  convertStringToArrayOfInt(command, command_array, AOK_COMMAND_BIT_ARRAY_SIZE);
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doAOKTribitSend(command_array);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // disrupting most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doAOKTribitSend(int *command_array) {

  if (command_array == NULL) {
    errorLog("doAOKTribitSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Starting (AGC) bits:
  transmitHigh(AOK_AGC1_PULSE);
  transmitLow(AOK_AGC2_PULSE);

  // Transmit command:
  for (int i = 0; i < AOK_COMMAND_BIT_ARRAY_SIZE; i++) {

      // If current bit is 0, transmit HIGH-LOW-LOW (100):
      if (command_array[i] == 0) {
        transmitHigh(AOK_PULSE_SHORT);
        transmitLow(AOK_PULSE_LONG);
      }

      // If current bit is 1, transmit HIGH-HIGH-LOW (110);
      if (command_array[i] == 1) {
        transmitHigh(AOK_PULSE_LONG);
        transmitLow(AOK_PULSE_SHORT);
      }   
   }

  // Radio silence at the end.
  // It's better to go a bit over than under minimum required length:
  transmitLow(AOK_RADIO_SILENCE);
  
  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(AOK_COMMAND_BIT_ARRAY_SIZE);
    Serial.println(" bits.");
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH);
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW);
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
int convertStringToInt(String s) {
  char carray[2];
  int i = 0;
  
  s.toCharArray(carray, sizeof(carray));
  i = atoi(carray);

  return i;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void convertStringToArrayOfInt(String command, int *int_array, int command_array_size) {
  String c = "";

  if (int_array == NULL) {
    errorLog("convertStringToArrayOfInt(): Array pointer was NULL, cannot continue.");
    return;
  }
 
  for (int i = 0; i < command_array_size; i++) {
      c = command.substring(i, i + 1);

      if (c == "0" || c == "1") {
        int_array[i] = convertStringToInt(c);
      } else {
        errorLog("convertStringToArrayOfInt(): Invalid character " + c + " in command.");
        return;
      }
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
