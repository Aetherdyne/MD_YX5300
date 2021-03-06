// Simple MP3 player using the MD_YX5300 library.
//
// MP3 player has the following functions:
// - Start/pause track playback with single switch (momentary on type)
// - Play next track with single switch (momentary on type)
// - Volume control using potentiometer
// - Run state shown using LED indicator
//
// Implemented using synchronous calls and kept as simple as possible.
// All tracks need to be placed in folder PLAY_FOLDER (defined below)
//
// Library dependencies:
// MD_UISwitch can be found at https://github.com/MajicDesigns/MD_UISwitch
//

#include <MD_YX5300.h>
#include <MD_UISwitch.h>

// Connections for serial interface to the YX5300 module
const uint8_t ARDUINO_RX = 4;    // connect to TX of MP3 Player module
const uint8_t ARDUINO_TX = 5;    // connect to RX of MP3 Player module

const uint8_t SW_PLAY_PAUSE = 2; // play/pause toggle digital pin, active low (PULLUP)
const uint8_t SW_PLAY_NEXT = 3;  // play next track digital pin, active low (PULLUP)
const uint8_t PIN_LED = 4;       // LED to show status
const uint8_t POT_VOLUME = A0;   // volume control pot analog pin

const uint8_t PLAY_FOLDER = 1;   // tracks are all placed in this folder


#define DEBUG 0 // enable/disable debug output

#ifdef DEBUG
#define PRINT(s,v)    { Serial.print(F(s)); Serial.print(v); }
#define PRINTX(s,v)   { Serial.print(F(s)); Serial.print(v, HEX); }
#define PRINTS(s)     { Serial.print(F(s)); }
#else
#define PRINT(s,v)
#define PRINTX(s,v)
#define PRINTS(s)
#endif


// Define global variables
MD_YX5300 mp3(ARDUINO_RX, ARDUINO_TX);
MD_UISwitch_Digital swPause(SW_PLAY_PAUSE);
MD_UISwitch_Digital swNext(SW_PLAY_NEXT);

bool playerPause = true;  // true if player is currently paused


void processVolume(bool bForce = false)
// read the volume pot and set the volume if it has changed
{
  static uint8_t vol;   // current audio volume
  uint16_t pot = analogRead(POT_VOLUME);
  uint8_t newVolume = map(analogRead(POT_VOLUME), 0, 1023, 0, mp3.volumeMax());
  
  if (newVolume != vol || bForce)
  {
    PRINT("\nSetting volume ", newVolume);
    vol = newVolume;
    bool b = mp3.volume(vol);
    PRINT(" result ", b);
  }
}

void processPause(bool bForce = false)
// read the pause switch and act if it has been pressed
{
  MD_UISwitch::keyResult_t k = swPause.read();

  if (k == MD_UISwitch::KEY_PRESS || bForce)
  {
    bool b;

    if (!bForce) playerPause = !playerPause;
    PRINT("\nSwitching to ", playerPause ? F("PAUSE") : F("PLAY"));
    if (playerPause) b = mp3.playPause(); else b = mp3.playStart();
    PRINT(" result ", b);
  }
}

void processNext(bool bForce = false)
// read the next switch and act if it has been pressed
{
  MD_UISwitch::keyResult_t k = swNext.read();

  if (playerPause) return;    // this command makes no sense if paused

  if (k == MD_UISwitch::KEY_PRESS || bForce)
  {
    bool b;

    PRINTS("\nPlaying next");
    b = mp3.playNext();
    PRINT(" result ", b);
  }
}

void setStatusLED(void)
// set the status led - on for running, off for paused
{
  if (playerPause)
    digitalWrite(PIN_LED, LOW);
  else
    digitalWrite(PIN_LED, HIGH);
}

void setup()
{
#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_YX5300 Simple Player]");

  // set the hardware pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(POT_VOLUME, INPUT);

  // initialize global libraries
  mp3.begin();
  mp3.setSynchronous(true);
  mp3.playFolderRepeat(PLAY_FOLDER);
  processVolume(true);    // force these to set up the hardware
  processPause(true);

  // Set up the switches modes - only simple switching
  swPause.begin();
  swPause.enableDoublePress(false);
  swPause.enableRepeat(false);
  swPause.enableLongPress(false);
  
  swNext.begin();
  swNext.enableDoublePress(false);
  swNext.enableRepeat(false);
  swNext.enableLongPress(false);
}

void loop()
{
  mp3.check();        // run the mp3 receiver
  processVolume();    // set the volume if required
  processPause();     // process the Pause switch
  processNext();      // provess the Next switch
  setStatusLED();     // set the status LED to current status
}
