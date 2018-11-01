/*
******************************************************************************************************************************************************************
*
* A-OK 433.92MHz motorized window shades
* Control code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* AC114-01B is the only remote control tested.
* 
* https://www.a-okmotors.com/en/
* 
* This is an incomplete protocol implementation for the A-OK window shades.
* I don't personally have any of these motors in use, just happened to
* stumble upon some AM25-1.2/30-MEL-P units and quickly edited my Markisol
* protocol code to control it.
* 
* The motors themselves are of sturdy build and amazingly quiet in operation.
* If there is anything obviously negative to state about them, it's the short
* power cable. Since they run on AC power, an extension cord hanging from
* the top of your window looks kind of ugly (compared to thin DC 5/12/24V
* cables).
* 
* 
* PROTOCOL DESCRIPTION
* 
* Two remote controls and motors are not enough data to understand the protocol,
* but here's what I've figured out so far. Unless I'm completely mistaken, the
* whole sequence seems encoded/ciphered with something, possibly XOR.
* 
* UP and DOWN buttons send two different commands for some reason, listed below
* as AOK_UP/DOWN_1/2 and AOK_AFTER_UP_DOWN_1/2. However, the latter command
* (AOK_AFTER_UP_DOWN_1/2) would not seem to be necessary.
* 
* Tri-state bits are used.
* A single command is: 2 AGC bits + 65 command tribits + short radio silence
* Swapping HIGH and LOW does not work with these shades.
* 
* All sample counts below listed with a sample rate of 44100 Hz
* (sample count / 44100 = microseconds).
*
* Starting (AGC) bits:
* HIGH of approx. 234 samples = 5306 us
* LOW of approx. 225 samples = 5102 us
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
* What I've figured of the command bits so far:
* 24 bits for (unique) remote control ID, hard coded in remotes
* 4 bits for command: DOWN = ????, UP = ????, STOP = ????, SETTING = ????
* Other 37 bits unknown as of now
* = 65 bits in total
* 
* End with LOW radio silence of (minimum) 11 samples = 249 us
*
******************************************************************************************************************************************************************
*/



// Remote 1:
#define AOK_DOWN_1              "10101110010100000001110000000011011111110111111111011110010011100"
#define AOK_UP_1                "10101110010100000001110000000011011111110111111111111010011010100"
#define AOK_AFTER_UP_DOWN_1     "10101110010100000001110000000011011111110111111111101101110111011"
#define AOK_STOP_1              "10101110010100000001110000000011011111110111111111101110010111100"
#define AOK_SETTING_1           "10101110010100000001110000000011011111110111111111010110010001100"

// Remote 2:
#define AOK_DOWN_2              "10101110010100000000110011100110111111110111111111011110001000100"
#define AOK_UP_2                "10101110010100000000110011100110111111110111111111010101000110010"
#define AOK_AFTER_UP_DOWN_2     "10101110010100000000110011100110111111110111111111101101101100011"
#define AOK_STOP_2              "10101110010100000000110011100110111111110111111111101110001100100"
#define AOK_SETTING_2           "10101110010100000000110011100110111111110111111111010110000110100"


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
#define AOK_AGC2_PULSE                   5090  // 225 samples
#define AOK_RADIO_SILENCE                300   // 11 samples

#define AOK_PULSE_SHORT                  155   // 7 samples, used for LOW
#define AOK_PULSE_MEDIUM                 380   // 17 samples, used for HIGH-HIGH
#define AOK_PULSE_LONG                   650   // 31 samples, used for HIGH-HIGH-HIGH

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
  //sendAOKCommand(AOK_PAIR_2);
  //while (true) {} // Stop after pairing, you can use UP/STOP/DOWN commands afterwards

  // Send the command:
  sendAOKCommand(AOK_UP_1);
  delay(5000);
  sendAOKCommand(AOK_STOP_1);
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
  // which will disrupt most appliances operating on the 433.92MHz band:
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
  transmitWaveformHigh(AOK_AGC1_PULSE);
  transmitWaveformLow(AOK_AGC2_PULSE);

  // Transmit command:
  for (int i = 0; i < AOK_COMMAND_BIT_ARRAY_SIZE; i++) {

      // If current bit is 0, transmit LOW-LOW-HIGH-HIGH-LOW (00110):
      if (command_array[i] == 0) {
        transmitWaveformLow(AOK_PULSE_MEDIUM);
        transmitWaveformHigh(AOK_PULSE_MEDIUM);
        transmitWaveformLow(AOK_PULSE_SHORT);
      }

      // If current bit is 1, transmit HIGH-HIGH-HIGH-LOW:
      if (command_array[i] == 1) {
        transmitWaveformHigh(AOK_PULSE_LONG);
        transmitWaveformLow(AOK_PULSE_SHORT);
      }   
   }

  // Radio silence at the end.
  // It's better to rather go a bit over than under required length.
  transmitWaveformLow(AOK_RADIO_SILENCE);
  
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
void transmitWaveformHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW); // Digital pin low transmits a high waveform
  //PORTB = PORTB D13low; // If you wish to use faster PORTB commands instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH); // Digital pin high transmits a low waveform
  //PORTB = PORTB D13high; // If you wish to use faster PORTB commands instead
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

      if (c == "0" || c == "1" || c == "2" || c == "3") { // 2 and 3 are allowed for other projects of mine (not used by this code)
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
