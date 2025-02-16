//
// 9-button GKOS input method for mobile chorded keyboard with joystick
// Losely based on the outdated library of Seppo Tiainen, 28-Mar-2010
// Changed implementation to interface with Leonardo Keyboard library;
// Reduced memory consumption from 52 to 30%.
// Key assignments changed as per the letter frequency.
// v 4.0 23-Oct-2017
// This work is public domain
//

#include <Mouse.h>
#include <Keyboard.h>

boolean keyboardActive = false;                 // Safety feature, in case programming goes bananas
const bool Serial_report = false;               // Set to true for serial debugging 

//  
// Pin assignment
//
const int pot_sensorPins[] = {A3, A2};
const int GKOSPins[] = {2, 3, 4, 5, 6, 7};      // reading pins for 6 GKOS buttons
//const int GKOSPins[] = {2, 3, 4, 5, 6, 7};      // reading pins for 6 GKOS buttons
//const int GKOSPins[] = {10, 9, 3, 2, 1, 0};      // reading pins for 6 GKOS buttons Leonardo Chiquita 
const int MousePins[] = {8, 9, 19};             // reading pins for 3 mouse buttons
const int LEDPins[] = {14, 15, 16};             // LED pins for indicators


//Define variables to act as a mouse
// set pin numbers for the six buttons:
// Must coincide with gkos pins as shown below
const int upButton = GKOSPins[0];
const int downButton = GKOSPins[4];
const int leftButton = GKOSPins[1];
const int rightButton = GKOSPins[3];
const int mouseButton = GKOSPins[2];
const int mouseleftButton = GKOSPins[5];
//Leonardo chiquita
//const int upButton = 0;
//const int downButton = 9;
//const int leftButton = 1;
//const int rightButton = 3;
//const int mouseButton = 2;
//const int mouseleftButton = 10;
int range = 1;              // output range of X or Y movement; affects movement speed
int responseDelay = 1;     // response delay of the mouse, in ms
int distMouse2xy = 0;     // para detectar cmabios en el movimiento pedido al mouse y acelerarlo en consecuencia
bool myMouseMode = false;            //mouse mode 0 keyboard mode 1 mouse mode


//
// arrays are used to map values for each of four potentiometers
//
int pot_Low[]= {0, 0};
int pot_Middle[]= {512, 512};  
int pot_High[]= {1023, 1023};  
int pot_Calibrated_Low[]= {120, -120};  
int pot_Calibrated_High[]= {-120, 120};  
signed char pot_Position[] = {(signed char)0, (signed char)0, (signed char)0};

//
// Character definition
//

// Mouse moves
//const char Mouse_Move_Horizontal[] = {(const char)KEY_HOME, (const char)KEY_LEFT_ARROW, ' ', (const char)KEY_RIGHT_ARROW, (const char)KEY_END};
//const char Mouse_Move_Vertical[] = {(const char)KEY_PAGE_UP, (const char)KEY_UP_ARROW, ' ', (const char)KEY_DOWN_ARROW, (const char)KEY_PAGE_DOWN};
const char Mouse_Move_Horizontal[] = {(const char)KEY_LEFT_ARROW, (const char)KEY_LEFT_ARROW, ' ', (const char)KEY_RIGHT_ARROW, (const char)KEY_RIGHT_ARROW};
const char Mouse_Move_Vertical[] = {(const char)KEY_UP_ARROW, (const char)KEY_UP_ARROW, ' ', (const char)KEY_DOWN_ARROW, (const char)KEY_DOWN_ARROW};

// GKOS Reference number based on Chord value
const static int GKOS_Chord2_gRef[] = {
   0, 1,21,15, 3, 4,19,44,
   9,48,35,16,27,28,23,45,
   5,34,61, 6,36,33,13,46,
  20, 8, 7,49,10,29,32,52,
  18,38,37,17,51,31,22,47,
  12,24,25,56,26,62,59,53,
  14,30, 2,39,11,60,50,54,
  40,41,42,58,43,57,55,63};

// Parallel GKOS character sets for lower case, caps, shift (upper case), numbers and symbols
//const static char GKOS_Char[]={
// ' ','a','b','c','d','e','f','g',
// 'h','i','j','k','l','m','n','o',
// 'p','q','r','s','t','u','v','w',
// 'x','y','z','^','[',']','@','=',
//'\\','`','-','\"',',','.','?','/',
// ' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
// (const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
// (const char)KEY_INSERT,(const char)KEY_TAB,(const char)KEY_RETURN,'F','R',(const char)KEY_RIGHT_SHIFT,'S','L'};
const static char GKOS_Char[]={
 ' ','a','b','c','d','e','f','g',
 'h','i','j','k','l','m','n','o',
 'p','q','r','s','t','u','v','w',
 'x','y','z','^','[',']','@','=',
'/','`','-','\'',',','.','?','\\',
 ' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
 (const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
 (const char)KEY_INSERT,(const char)KEY_TAB,(const char)KEY_RETURN,'F',';',(const char)KEY_RIGHT_SHIFT,'S','L'};

const static char GKOS_Caps[]={
 ' ','a','b','c','d','e','f','g',
 'h','i','j','k','l','m','n','o',
 'p','q','r','s','t','u','v','w',
 'x','y','z','&','{','}','#','+',
 '|','~','_','\"',';',':','!','&',
 ' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
 (const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
 (const char)KEY_INSERT,(const char)KEY_TAB,(const char)KEY_RETURN,'F',';',(const char)KEY_RIGHT_SHIFT,'S','L'};
//const static char GKOS_Caps[]={
// ' ','a','b','c','d','e','f','g',
// 'h','i','j','k','l','m','n','o',
// 'p','q','r','s','t','u','v','w',
// 'x','y','z','&','{','}','#','+',
// '|','~','_','\'',';',':','!','&',
// ' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
// (const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
// (const char)KEY_INSERT,(const char)KEY_TAB,(const char)KEY_RETURN,'F','R',(const char)KEY_RIGHT_SHIFT,'S','L'};

const static char GKOS_SYMB[]={
' ','1','-','3',')','5','=','8',
'7','4','9','&','(','&','#','+',
'%','^','6','*','0','2','|','$',
'[','<','{','&','{','}','#','+',
'|','>','_','\"',';',':','!','&',
' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
(const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
'+',(const char)KEY_TAB,(const char)KEY_RETURN,(const char)KEY_INSERT,'/',(const char)KEY_RIGHT_SHIFT,'S','L'};

//const static char GKOS_SYMB[]={
//  ' ','2','e','<','e','1','e','7',
//  '#','4','e','>','3','e','0','5',
//  ')','8','6','e','e','e','%','=',
//  '9','(','^','&','{','}','[',']',
//  '|','&','-','+',',','.','*','/',
//  ' ',(const char)KEY_RIGHT_ARROW,'V',(const char)KEY_END,(const char)KEY_BACKSPACE,(const char)KEY_LEFT_ARROW,'C',(const char)KEY_HOME,
//  (const char)KEY_UP_ARROW,(const char)KEY_PAGE_UP,(const char)KEY_PAGE_DOWN,(const char)KEY_DOWN_ARROW,(const char)KEY_ESC,(const char)KEY_RIGHT_CTRL,(const char)KEY_LEFT_ALT,(const char)KEY_DELETE,
//  (const char)KEY_INSERT,(const char)KEY_TAB,(const char)KEY_RETURN,'F','R',(const char)KEY_RIGHT_SHIFT,'S','L'};


//
// Buttons data
//
long  GKOS_time = 0;
long  GKOS_previousTime = 0;
long  GKOS_autoRepeat = 0;   // Repeat the character if chord pressed long enough (0 or 1)
int  GKOS_presentChord = 0;  // The chord of the keypad at present
int  GKOS_previousChord = 0; // To compare chord change
int  GKOS_autoCounter = 0;   // Typamatic delay counter (n x 10 ms)
int  GKOS_firstChord = 0;    // First chord of a Chordon, if any
int  GKOS_firstCounter = 0;  // Count to detect a separate first chord in a Chordon
int  GKOS_gChord = 0;        // GKOS final chord value (0...63) for the character
bool  GKOS_gNew = false;     // a new character is expected soon because new keys were pressed
int  GKOS_gRef = 0;          // Character index within the set (0...63)

int  GKOS_mode = 0;          // mode: 0 - normal, 1 - shift, 2 - caps lock, 3 - sym, 4 - num lock, 5 - Mouse Mode
char GKOS_output = ' ';

//
// Control keys
//
bool CTRL_Requested = false;
bool ALT_Requested = false;
bool WIN_Requested = false;

//
// Setup code here, to run once:
//
void setup()
{
  if (Serial_report) Serial.begin(9600);

  for ( int i = 0; i < 3; i++)
  {
    pinMode(LEDPins[i], OUTPUT);
    digitalWrite(LEDPins[i], HIGH);
  }
  Calibrate_Pots();
  for ( int i = 0; i < 6; i++) pinMode(GKOSPins[i], INPUT_PULLUP);
  for ( int i = 0; i < 3; i++) pinMode(MousePins[i], INPUT_PULLUP);
  keyboardActive = !(ReadPinBool(MousePins[0]) || ReadPinBool(MousePins[1]));
  if ( !keyboardActive) return;
  Mouse.begin();
  Keyboard.begin();
  delay( 500);
  WriteLEDs( LOW, LOW, LOW);
}

//
// runs continuously
//
void loop()
{
  bool output_ready;
  if ( keyboardActive)
  {
    if( GKOS_mode == 5)
    {
      gMouse();
    } else {

        if (GKOS_Read_Buttons())
        {
                output_ready = GKOS_Decoder();
                if (output_ready) SendButtons();
                if (output_ready && Serial_report) Serial.println( "Button code: " + String( GKOS_output) + " gref:" + String( GKOS_gRef) + " Chord:" + String( GKOS_gChord));
        }   
        if (Serial_report) delay(500);
        else delay(20);
    }
///   if( GKOS_mode >= 3)
   if( GKOS_mode < 3)
   {
     Read_Mouse_Pots();
     delay(50);
   }
   else
   {
     Read_Navigation_Pots();
   }
  }
  return;
}


//void loop() {
//    //Serial.println(myMouseMode);
//    //Serial.println("loop");
//    //if ( (mouseMode(2)) == 0 ) {
//    if ( !myMouseMode ) {
//        gEntry = gkos.entry(); // Will return empty immediately if no entry
//        //if (gEntry != 0){gPrint();}
//        if (strcmp(gEntry, "") != 0) {
//          gPrint();
//          gLastEntry = gEntry;
//        }
//    } else {
//      gMouse();
//    };
//  // Add your other code here
//}



//
// Calibrates middle points in all 4 pots, only needs to run once
//
void Calibrate_Pots()
{
  int pin, data;
  for( int i=0; i<2; i++)
  {
    pin = pot_sensorPins[i];
    data = analogRead(pin);
    for( int j=0; j<4; j++)
    {
      delay(10);
      data += analogRead(pin);
    }
    pot_Middle[i] = data/5;
    pot_Position[i] = (signed char)pot_Middle[i];
  }
  return;
}

//
// Reads the pots and sends mouse commands
//
void Read_Mouse_Pots()
{
  int data;
  bool button_L = ReadPinBool( MousePins[0]);
  bool button_R = ReadPinBool( MousePins[1]);
  bool button_M = button_L && button_R;
  bool button_Scroll = ReadPinBool( MousePins[2]);
  if(  button_M)
  {
    button_L = false;
    button_R = false;
  }
  if( button_L && !Mouse.isPressed(MOUSE_LEFT)) Mouse.press(MOUSE_LEFT);
  if( !button_L && Mouse.isPressed(MOUSE_LEFT)) Mouse.release(MOUSE_LEFT);
  if( button_R && !Mouse.isPressed(MOUSE_RIGHT)) Mouse.press(MOUSE_RIGHT);
  if( !button_R && Mouse.isPressed(MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT);
  if( button_M && !Mouse.isPressed(MOUSE_MIDDLE)) Mouse.press(MOUSE_MIDDLE);
  if( !button_M && Mouse.isPressed(MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);

  for( int i=0; i<2; i++)
  {
    data = ReadAnaloguePin( i);
    if( -100<data && data<100)
//      pot_Position[i] = (signed char)(data / 3); 
      pot_Position[i] = (signed char)(data / 5); 
    else
      pot_Position[i] = (signed char)(data);
//    if( button_Scroll) pot_Position[i] = pot_Position[i] / 3; 
    if( button_Scroll) pot_Position[i] = pot_Position[i] / 5; 
  }

  if( button_Scroll)
    Mouse.move(pot_Position[2], pot_Position[2], -pot_Position[0]);
  else
    Mouse.move(pot_Position[1], pot_Position[0], pot_Position[2]);
//  if (Serial_report)
//  {
//    Serial.println( "Mouse pointer: " + String(pot_Position[0]) + " " + String(pot_Position[1]) + " " + String(pot_Position[2]));
//    Serial.println( "Mouse buttons: " + String(button_L) + " " + String(button_M) + " " + String(button_R) + " " + String(button_Scroll));
//  }
  return;
}

//
// Reads the pots and sends navigation commands
//
void Read_Navigation_Pots()
{
  int data;
  int keySelection = 0;
  bool button_Shift = ReadPinBool( MousePins[0]);
  bool button_Control = ReadPinBool( MousePins[1]);

  data = DecodeAnalogueToKey( ReadAnaloguePin( 1));
  if( data != 0)
  {
    if( button_Shift) Keyboard.press(KEY_LEFT_SHIFT);
    if( button_Control) Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.write( Mouse_Move_Horizontal[data+2]);
    delay( 100);
    if( button_Control) Keyboard.release(KEY_LEFT_CTRL);
    if( button_Shift) Keyboard.release(KEY_LEFT_SHIFT);
    return;
  }
  data = DecodeAnalogueToKey( ReadAnaloguePin( 0));
  if( data != 0)
  {
    if( button_Shift) Keyboard.press(KEY_LEFT_SHIFT);
    if( button_Control) Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.write( Mouse_Move_Vertical[data+2]);
    delay( 100);
    if( button_Control) Keyboard.release(KEY_LEFT_CTRL);
    if( button_Shift) Keyboard.release(KEY_LEFT_SHIFT);
    return;
  }
}

//
// Reads the button groups, processes them into the press codes
//
bool GKOS_Read_Buttons()
{
  int button_weight = 1;
  int pin = 0;
  int i, j;

  // Return empty immediately if 10 ms not elapsed
  GKOS_time = millis();
  if ((GKOS_time - GKOS_previousTime) < (10 + 64 * GKOS_autoRepeat)) return false;

  // Read buttons and form a code 
  GKOS_presentChord = 0;
  for( int i=0; i<6; i++)
  {
    pin = ReadPin( GKOSPins[i]);
    GKOS_presentChord += pin * button_weight;
    button_weight += button_weight; 
  }

  // any buttons pressed
  if (GKOS_presentChord != 0)
  {
    // Chord pressed long enough for autorepeat
    GKOS_autoCounter++;
    if (GKOS_autoCounter >= 100)
    {
      GKOS_autoCounter = 99;
      GKOS_autoRepeat = 1;
    }

    // Chord pressed long enough to be first chord in Chordon
    GKOS_firstCounter++;
    if (GKOS_firstCounter > 25)
    {
      GKOS_firstCounter = 25;
      if (GKOS_firstChord == 0) GKOS_firstChord = GKOS_presentChord;
    }
  }

  // New key(s) pressed, expect a new character soon
  if (GKOS_presentChord > GKOS_previousChord)
  {
    GKOS_gNew = true;
    GKOS_autoCounter = 0;
    GKOS_autoRepeat = 0;
    GKOS_firstCounter = 0;
  }

  // key(s) relased
  if (GKOS_presentChord < GKOS_previousChord)
  {
    GKOS_autoCounter = 0;
    GKOS_autoRepeat = 0;
    GKOS_firstCounter = 0;
    GKOS_gChord = GKOS_previousChord;  // the chord before the first release of key(s) is what we are after
    GKOS_previousChord = GKOS_presentChord;  
    GKOS_previousTime = GKOS_time;
    if (!GKOS_gNew) return false;
    GKOS_gNew = false; // next key releases will no more represent chords for characters
    return true; // a real chord entered!
  }

  // no buttons pressed
  if (GKOS_presentChord == 0)
  {
    GKOS_firstChord = 0; 
    GKOS_firstCounter = 0;
  }
  GKOS_previousChord = GKOS_presentChord; // to be able to compare next time
  GKOS_previousTime = GKOS_time;

  if (!GKOS_autoRepeat) return false;
  GKOS_gChord = GKOS_previousChord;
  GKOS_firstCounter = 0;
  GKOS_firstChord = 0;
  return true;
}

bool GKOS_Decoder()
{
  // If it is a Chordon, send it here
  // Seppo Tiainen did not finish this
  /*     if (_firstChord != 0){_firstChord = 0; _firstCounter = 0; // TEMP!!!!
        return "-";} // TODO: combine two characters and return directly */
  GKOS_firstChord = 0;
  GKOS_firstCounter = 0;  // in any case, delete the first char of Chordon
    
  // Change between Character sets when Shift/CAPS, 123abc or SYMB are pressed
  GKOS_gRef = GKOS_Chord2_gRef[ GKOS_gChord];
  switch (GKOS_mode)
  {
    case 0: // Normal
      GKOS_output = GKOS_Char[GKOS_gRef];
      break;
    case 1: // Shift
    case 2: // Caps 
      GKOS_output = GKOS_Caps[GKOS_gRef];
      break;
    //case 5: // Mouse
      //break;
    default: // Symb
      GKOS_output = GKOS_SYMB[GKOS_gRef];
      break;
  } // switch
  return true;
}

void SendButtons()
{
  if( SubstituteShift((char)KEY_RIGHT_SHIFT)) return;
  if( SubstituteCtrl((char)KEY_RIGHT_CTRL)) return;
  if( SubstituteAlt((char)KEY_LEFT_ALT)) return;
  if( SubstituteWin((char)KEY_RIGHT_GUI)) return;
  if( SubstituteRusLat()) return;
  if( SubstituteFunction()) return;
  if( SubstituteSymbol()) return;
  if( SubstituteABC123()) return;
  if( SubstituteControlMetacode( 'Z', 'z')) return;
  if( SubstituteControlMetacode( 'X', 'x')) return;
  if( SubstituteControlMetacode( 'C', 'c')) return;
  if( SubstituteControlMetacode( 'V', 'v')) return;
  if( SubstituteMetacode( 'O', " || ")) return;
  if( SubstituteMetacode( 'A', " && ")) return;
  if( SubstituteMouse()) return;
  if( CTRL_Requested) Keyboard.press( KEY_RIGHT_CTRL);
  if( ALT_Requested) Keyboard.press( KEY_LEFT_ALT);
  if( WIN_Requested) Keyboard.press( KEY_LEFT_GUI);
  Keyboard.write( GKOS_output);
  if( WIN_Requested) Keyboard.release( KEY_LEFT_GUI);
  if( ALT_Requested) Keyboard.release( KEY_LEFT_ALT);
  if( CTRL_Requested) Keyboard.release( KEY_RIGHT_CTRL);
  if( GKOS_mode == 1)
  {
    GKOS_mode = 0;
    Keyboard.release( KEY_RIGHT_SHIFT);
  }
  if( GKOS_mode == 3)
  {
    GKOS_mode = 0;
  }  
  CTRL_Requested = false;
  ALT_Requested = false;
  WIN_Requested = false;
}

//
// Reads a pin, returns the reading
//
int ReadPin( int i)
{
  if ( digitalRead( i) == LOW) return 1;
  return 0;
}

//
// Reads a pin, returns the reading
//
bool ReadPinBool( int i)
{
  return digitalRead(i) == LOW;
}

//
// Reads an analogue pin, returns calibrated value
//
int ReadAnaloguePin( int i)
{
  int pin = pot_sensorPins[i];
  int data = analogRead(pin);
  if( data < pot_Middle[i])
    data = map(data, pot_Low[i], pot_Middle[i], pot_Calibrated_Low[i], 0);
  else
    data = map(data, pot_Middle[i], pot_High[i],  0, pot_Calibrated_High[i]);
  if( -3<data && data<3) data = 0;
  return data;
}

//
// Decodes mouse position into keys
//
int DecodeAnalogueToKey( int i)
{
  if( i < -100) return -2;
  if( i > 100) return 2;
  if( i < -10) return -1;
  if( i > 10) return 1;
  return 0;
}

//
// Writes to LEDs
//
void WriteLEDs( bool v1, bool v2, bool v3)
{
  digitalWrite(LEDPins[0], v1);
  digitalWrite(LEDPins[1], v2);
  digitalWrite(LEDPins[2], v3);
}

//
// Substitutes a mata-code with a key code; returns true if further processing not required
//
bool SubstituteMetacode( char code, String meta)
{
  if( code != GKOS_output) return false;
  Keyboard.print( meta);
  return true;
}

//
// Substitutes a mata-code with a key code; presses left Ctrl to pass the code
//
bool SubstituteControlMetacode( char code, char meta)
{
  if( code != GKOS_output) return false;
  Keyboard.press( KEY_LEFT_CTRL);
  Keyboard.write( meta);
  Keyboard.release(KEY_LEFT_CTRL);
  CTRL_Requested = false; 
  return true;
}

//
// Sets Shift key
//
bool SubstituteShift( char code)
{
  if( GKOS_output != code) return false;
  switch( GKOS_mode)
  {
    case 0:
      GKOS_mode = 1;
      Keyboard.press( code);
      return true;
    case 1:
      GKOS_mode = 2;
      return true;
    case 2:
      GKOS_mode = 0;
      Keyboard.release( code);
      return true;
    default:
      return true;
  }
}

//
// Sets Control key
//
bool SubstituteCtrl( char code)
{
  if( GKOS_output != code) return false;
  CTRL_Requested = true;
  return true;
}

//
// Sets Alt key
//
bool SubstituteAlt( char code)
{
  if( GKOS_output != code) return false;
  ALT_Requested = true;
  return true;
}

//
// Sets Win key
//
bool SubstituteWin( char code)
{
  if( GKOS_output != code) return false;
  WIN_Requested = true;
  return true;
}

//
// Changes between Rus and Lat by pressing and releasing Shift-Control
//
bool SubstituteRusLat()
{
  if( GKOS_output != 'R') return false;
  Keyboard.press( KEY_LEFT_ALT);
  Keyboard.press( KEY_RIGHT_SHIFT);
  //Keyboard.press( KEY_RIGHT_CTRL);
  //Keyboard.release( KEY_RIGHT_CTRL);
  Keyboard.release( KEY_RIGHT_SHIFT);
  Keyboard.release( KEY_LEFT_ALT);
  return true;
}

//
// Sets Function mode
//
bool SubstituteFunction()
{
  if( GKOS_output != 'F') return false;
  return true;
}

//
// Sets Symbol mode
//
bool SubstituteSymbol()
{
  if( GKOS_output != 'S') return false;
  switch( GKOS_mode)
  {
    case 1:
    case 2:
      Keyboard.release(KEY_RIGHT_SHIFT);
      GKOS_mode = 3;
      return true;
    case 3:
      GKOS_mode = 0;
      return true;
    default:
      GKOS_mode = 3;
      return true;
  }
}

//
// Sets Num-Letter mode
//
bool SubstituteABC123()
{
  if( GKOS_output != 'L') return false;
  switch( GKOS_mode)
  {
    case 1:
      Keyboard.release(KEY_RIGHT_SHIFT);
      GKOS_mode = 5;
      return true;      
    case 2:
      Keyboard.release(KEY_RIGHT_SHIFT);
      GKOS_mode = 3;
      return true;
    case 4:
      GKOS_mode = 0;
      return true;
    case 5:
      GKOS_mode = 0;
      return true;   
    default:
      GKOS_mode = 4;
      return true;
  }
}


//
// Sets Mouse mode
//
bool SubstituteMouse()
{
  if( GKOS_mode != 5) return false;
  return true;
}

void gMouse() {  //Para cuando funciona como mouse
  // read the buttons:
  int upState = digitalRead(upButton);
  int downState = digitalRead(downButton);
  int rightState = digitalRead(rightButton);
  int leftState = digitalRead(leftButton);
  int clickRightState = digitalRead(mouseButton);
  int clickLeftState = digitalRead(mouseleftButton);

  int totalPress = 0;


  // calculate the movement distance based on the button states:
  int  xDistance = (leftState - rightState) * range;
  int  yDistance = (upState - downState) * range;

  // if X or Y is non-zero, move:
  if ((xDistance != 0) || (yDistance != 0)) {
    Mouse.move(xDistance, yDistance, 0);
        //Esto es para gestionar la velocidad del mouse de acuerdo al tiempo que mantenemmos las mismas teclas apreatadas
        if (distMouse2xy == (2 * xDistance + yDistance))  {
          if (responseDelay >= 2) {responseDelay = responseDelay - 0.5;}
        } else {
          distMouse2xy = (2 * xDistance + yDistance);
          responseDelay = 30;
        }  
  }
  else {
      //If all are pressed change to keyboard mode mouseMode = 0
      totalPress = upState + downState + rightState + leftState + clickRightState + clickLeftState;
      responseDelay = 30;
      //Serial.println("gMouse");
      //Serial.println(totalPress);
      if (totalPress == 0) {
        myMouseMode = false;
        Mouse.release(MOUSE_ALL);
        delay(responseDelay * 30);
        Keyboard.begin(); Mouse.end();
        GKOS_mode = 0;
        return;
      }
  }

  // a delay so the mouse doesn't move too fast:
  delay(responseDelay);
  
  // if the mouse button is pressed:
  if (clickLeftState != HIGH) {
    // if the mouse is not pressed, press it:
    if (!Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.press(MOUSE_LEFT);
    }
  }
  // else the mouse button is not pressed:
  //if (clickLeftState == HIGH) {
  else {
    // if the mouse is pressed, release it:
    if (Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.release(MOUSE_LEFT);
    }
  }
  if (clickRightState != HIGH) {
    // if the mouse is not pressed, press it:
    if (!Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.press(MOUSE_RIGHT);
    }
  }
  // else the mouse button is not pressed:
  //if (clickRightState == HIGH) {
  else {
    // if the mouse is pressed, release it:
    if (Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.release(MOUSE_RIGHT);
    }
  }

}

/*
 *  AUXiLIAR org-mode emacs
| 1 |  8 |
|---+----|
| 2 | 16 |
|---+----|
| 4 | 32 |
|   |    |



|          | 12 | 13 | 29 | 49 | 37 |  30 | 21 | 17 | 10 |  20 | 34 | 33 | 51 |
|----------+----+----+----+----+----+-----+----+----+----+-----+----+----+----|
|          | 01 | 11 | 11 | 10 | 10 |  01 | 10 | 10 | 01 |  00 | 00 | 10 | 10 |
|          | 00 | 00 | 01 | 01 | 00 |  11 | 01 | 01 | 10 |  01 | 10 | 00 | 11 |
|          | 10 | 10 | 10 | 01 | 11 |  10 | 10 | 00 | 00 |  10 | 01 | 01 | 01 |
|----------+----+----+----+----+----+-----+----+----+----+-----+----+----+----|
| ABCD EN  |  ^ |  [ |  ] |  @ |  = |   \ |  ` |  - |  " |   , |  . |  ? |  / |
|----------+----+----+----+----+----+-----+----+----+----+-----+----+----+----|
| SHIFT EN |  & |  { |  } |  # |  + | pip |  ~ |  _ |  & | OJO |  : |  ! |  & |
|----------+----+----+----+----+----+-----+----+----+----+-----+----+----+----|
| SYMB EN  |  & |  { |  } |  # |  + | pip |  ~ |  _ |  ' |   ; |  : |  ! |  & |
|          |    |    |    |    |    |     |    |    |    |     |    |    |    |

OJO
Como va con shift al ; en ingles lo pone como :, al igual que al . lo pone como :

|----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|
|          | 11 | 10 | 10 | 10 | 01 | 10 | 01 | 00 | 00 | 10 | 01 | 10 | 10 | 01 | 11 | 11 |
|          | 00 | 01 | 00 | 00 | 00 | 01 | 10 | 10 | 01 | 11 | 11 | 01 | 01 | 10 | 10 | 01 |
|          | 10 | 10 | 11 | 01 | 10 | 00 | 00 | 01 | 10 | 01 | 10 | 01 | 11 | 11 | 01 | 10 |
|----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|
| ABCD ES  | '  | pi | ¿  | ?  | !  | -  | '  | .  | ,  | /  | }  | "  | ñ  |    |    | +  |
|          |    | pe |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
|----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|
| SHIFT ES | "  | °  | ¡  | !  | °  | _  | "  | :  | ;  | "  | ]  | #  | Ñ  |    |    | *  |
|----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|
| SYMB ES  | +  | :  | *  | e  | e  | e  | e  | .  | ,  |    |    | e  | -  |    | ¡  | (  |
|----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----|

Codigo elisp para el cuadrito "C-x e" luego del ultimo ")" de cada funcion 

(defun int-to-binary-string (i)
  "convert an integer into it's binary representation in string format"
  (let ((res ""))
    (while (not (= i 0))
      (setq res (concat (if (= 1 (logand i 1)) "1" "0") res))
      (setq i (lsh i -1)))
    (if (string= res "")
        (setq res "0"))
    res)
)

(defun binary-string-combokey(numero)
"string binario de 6 caracteres con el formato de las teclas de un chord de combokey"
   (setq strNumero (int-to-binary-string numero))
   (while (< (length  strNumero ) 6) (setq strNumero (concat "0" strNumero)))
   (setq res strNumero)
)

(defun print-combokey(numero)
   (setq strNumero (binary-string-combokey numero))
   (end-of-line)
   (insert (concat "|" (number-to-string numero)))
   (forward-line 1)
   (end-of-line)
   (insert (concat "|" (substring strNumero 5 6) (substring strNumero 2 3)))
   (forward-line 1)
   (end-of-line)
   (insert (concat "|" (substring strNumero 4 5) (substring strNumero 1 2)))
   (forward-line 1)
   (end-of-line)
   (insert (concat "|" (substring strNumero 3 4) (substring strNumero 0 1)))
)


   0, 1,21,15, 3, 4,19,44,
   9,48,35,16,27,28,23,45,
   5,34,61, 6,36,33,13,46,
  20, 8, 7,49,10,29,32,52,
  18,38,37,17,51,31,22,47,
  12,24,25,56,26,62,59,53,
  14,30, 2,39,11,60,50,54,
  40,41,42,58,43,57,55,63};


Los caracteres que no son letras sin contar los que estan en simbolos salvo excepciones coinciden en los 3 modos
(setq raros '(12 13 29 49 37 30 21 17 10 20 34 33 51))

(while raros
   (save-excursion (forward-line 1)(end-of-line)(print-combokey  (car raros)))
   (setq raros (cdr raros))
)
|12|13|29|49|37|30|21|17|10|20|34|33|51
|01|11|11|10|10|01|10|10|01|00|00|10|10
|00|00|01|01|00|11|01|01|10|01|10|00|11
|10|10|10|01|11|10|10|00|00|10|01|01|01

 * 
 * 
 * 
 */
