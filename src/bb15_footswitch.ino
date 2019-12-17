/*
 * For Arduino Micro  
 *
 */

#include <MIDI.h>
//#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
 
//LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack
   /*!
    @param      lcd_Addr[in] I2C address of the IO expansion module. For I2CLCDextraIO,the address can be configured using the on board jumpers.
    @param      En[in] LCD En (Enable) pin connected to the IO extender module
    @param      Rw[in] LCD Rw (Read/write) pin connected to the IO extender module
    @param      Rs[in] LCD Rs (Reset) pin connected to the IO extender module
    @param      d4[in] LCD data 0 pin map on IO extender module
    @param      d5[in] LCD data 1 pin map on IO extender module
    @param      d6[in] LCD data 2 pin map on IO extender module
    @param      d7[in] LCD data 3 pin map on IO extender module
   */

#define ledLOW  12
#define ledMID  11
#define ledHI   10

#define btnLOW  9
#define btnMID  8
#define btnHI   7

#define outLOW  A0
#define outMID  A1
#define outHI   A2

// MIDI 
#define MIDI_PC 11000000    //C0 PROGRAM CHANGE
#define MIDI_CC 10110000    //B0 CONTROL CHANGE

#define AMP_MIDI_CHANNEL 16 // MIDI_CHANNEL_OMNI

#define chLOW 1
#define chMID 2
#define chHI  3

int eeprom_addr = 0;

int btnSel = 0;
int lastBtnSel = 0;

int currentProgram = 0;   // MIDI

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

const byte Buttons[] = {btnHI, btnMID, btnLOW};
const byte LEDS[] = {ledLOW, ledMID, ledHI};
const byte Outs[] = {outLOW, outMID, outHI};

#define SIZE(x) sizeof(x)/sizeof(x[0])

template<class T, size_t N>
byte RadioButtons(T(&_buttons)[N], byte State)
{
  static byte Output = 0;

  for (byte i = 0; i < N; i++)
  {
    if (digitalRead(_buttons[i]) == State)
    {
      Output = i + 1;
      break;
    }
  }
  return Output;
}

//software serial
SoftwareSerial midiSerial(2, 3); // digital pins that we'll use for soft serial RX & TX
MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);
 
void setup() {

  Serial.begin(9600);
  
  //  Set MIDI baud rate:
  // midiSerial.begin(31250);

  MIDI.begin(AMP_MIDI_CHANNEL);  // Listen to all incoming messages
  
  for (byte i = 0, j = SIZE(Buttons); i < j; i++)
    pinMode(Buttons[i], INPUT_PULLUP);

  for (byte i = 0, j = SIZE(LEDS); i < j; i++)
    pinMode(LEDS[i], OUTPUT);

  for (byte i = 0, j = SIZE(Outs); i < j; i++)
    pinMode(Outs[i], OUTPUT);

  // activate LCD module
  /*
  lcd.begin (20,4); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0); // set cursor to 0,0
  lcd.print("08 JUPITER");
  lcd.setCursor(0, 3); // set cursor to 0,0
  lcd.print(">1  2 >3  4  5 >7  8");
  */
}

//  Send a three byte midi message  
void midiSend(char status, char data1, char data2) {
  midiSerial.write(status);
  midiSerial.write(data1);
  midiSerial.write(data2);
}

//  Send a two byte midi message  
void midiProg(char status, int data ) {
  midiSerial.write(status);
  midiSerial.write(data);
}

void loop() {
  // You can fix this part to only show the value if it has changed.
  btnSel = RadioButtons(Buttons, LOW);

  if (btnSel == 0) {
    EEPROM.get(eeprom_addr, btnSel);
    if (btnSel == 0)  btnSel = 1;
  }

  if (btnSel != lastBtnSel) {

    /*      
    lcd.setCursor (0,2);        // go to start of 2nd line
    lcd.print("CHANNEL: "); 
    lcd.print(btnSel); 
    */
    
    // clear LED pins
    for (byte i = 0, j = SIZE(LEDS); i < j; i++)
      digitalWrite(LEDS[i], LOW);
      
    digitalWrite(LEDS[btnSel-1], HIGH);
    
    // clear out pins
    for (byte i = 0, j = SIZE(Outs); i < j; i++)
      digitalWrite(Outs[i], LOW);
      
    // clear out pins
    for (byte i = 0, j = btnSel; i < j; i++)
      digitalWrite(Outs[i], HIGH);
    
    lastBtnSel = btnSel;

    // send MIDI signal to AMP
    switch (btnSel) {
      case 1:
        MIDI.send(midi::ProgramChange, chLOW, 0, AMP_MIDI_CHANNEL);
        break;
      case 2:
        MIDI.send(midi::ProgramChange, chMID, 0, AMP_MIDI_CHANNEL);
        break;
      case 3:
        MIDI.send(midi::ProgramChange, chHI, 0, AMP_MIDI_CHANNEL);
        break;
    }

    // write to EEPROM
    EEPROM.put(eeprom_addr, btnSel);
    
  } else {
   
  } 
     
}

