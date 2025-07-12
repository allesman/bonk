// all time values are in seconds and are only converted to millisecs (*1000) directly before they're used

//general
const int SOLENOID = 8;
const int BUTTON_MINUS = 2;
const int BUTTON_MODE = 4;
const int BUTTON_PLUS = 6;
const float DEFAULT_INTRO = 0.05;  // how long the solenoid gets power when the code wants to use it (after it looses power again, that's when it snaps back and the actual bonk happens)

const int NUM_MODES = 3;  // how many modes there are
int mode = 0;             // 0 = "metronom", 1 = "timer", 2 = "random"

// metronome
const float DEFAULT_BPM = 100;
float bpm = DEFAULT_BPM;

// timer
unsigned long startTime = 0;
const float DEFAULT_TIMER_DURATION = 3;
float customTimerDuration = DEFAULT_TIMER_DURATION;  // TODO: min value

// random
int randomMin = 1;
int randomMax = 20;
float randomTimer = random(randomMin, randomMax);

// button
bool modeButtonPressed = false;
bool plusButtonPressed = false;
bool minusButtonPressed = false;


void bonk() {
  /**
  Sends bonk signal to the pin that gives power to the solenoid.
  */
  digitalWrite(SOLENOID, HIGH);
  delay(DEFAULT_INTRO * 1000);
  digitalWrite(SOLENOID, LOW);
}

bool timerIsUp(float timerDuration) {
  // find out how much time has passed since last timer reset (in millisecs)
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - startTime;

  // calculate how much time should pass before timer is up (in millisecs)
  long timerThreshold = (timerDuration - DEFAULT_INTRO) * 1000;  // if we don't subtract INTRO here, the sound would be heard slightly too late (solenoid would pull back at the moment where it should already strike)

  return deltaTime >= timerThreshold;
}

void bonkAfterDelay(float delay) {
  // execute bonk as soon as the time since reset >= delay
  if (timerIsUp(delay)) {
    // Timer is up
    bonk();


    startTime = millis();
    if (mode == 2) { randomTimer = random(randomMin, randomMax); }  //only needed in "random" mode
  }
}

void setup() {
  // register solenoid pin as output
  pinMode(SOLENOID, OUTPUT);
  // register button pins and use internal pull-up resistor for them
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_PLUS, INPUT_PULLUP);
  pinMode(BUTTON_MINUS, INPUT_PULLUP);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
}

void loop() {

  int valueChangeByUser = 0;

  // READ BUTTON STATES
  int modeButtonState = digitalRead(BUTTON_MODE);
  int plusButtonState = digitalRead(BUTTON_PLUS);
  int minusButtonState = digitalRead(BUTTON_MINUS);

  // HANDLE MODE BUTTON
  // button pressed (button pin pulled LOW)
  if (modeButtonState == LOW) {
    // ensure button being pressed is only registered once (until it's released)
    if (!modeButtonPressed) {
      mode = (mode + 1) % NUM_MODES;  // change to next mode (magic math type shit)
      if (mode == 0) {
        bpm = DEFAULT_BPM;  // Reset BPM when switching back to the metronome mode
      }
      if (mode==1)
      {
        customTimerDuration = DEFAULT_TIMER_DURATION; // Reset timer when switching back to timer
      }
      startTime = millis();  // Reset the timer on mode change
      modeButtonPressed = true;
    }
  } else {  // button not pressed
    modeButtonPressed = false;
  }

  // HANDLE PLUS BUTTON
  if (plusButtonState == LOW) {
    // ensure button being pressed is only registered once (until it's released)
    if (!plusButtonPressed) {
      valueChangeByUser = 1;  // cause it's plus
      plusButtonPressed = true;
    }
  } else {  // button not pressed
    plusButtonPressed = false;
  }

  // HANDLE MINUS BUTTON
  if (minusButtonState == LOW) {
    // ensure button being pressed is only registered once (until it's released)
    if (!minusButtonPressed) {
      valueChangeByUser = -1;  // cause it's minus
      minusButtonPressed = true;
    }
  } else {  // button not pressed
    minusButtonPressed = false;
  }

  // EXECUTE LOGIC BASED ON MODE
  switch (mode) {
    case 0:
      // "metronome" mode (bonk with regular intervall chosen by user)
      bpm += valueChangeByUser * 10;  // update in case a change was requested
      bonkAfterDelay(60.0 / bpm);     //60 secs in min / beats per minute
      // TODO: allow user to customize
      break;
    case 1:
      // timer mode
      customTimerDuration += valueChangeByUser;              // update timer in case a change was requested
      customTimerDuration = max(1, customTimerDuration);     // prevent timer to be set to <= 0
      if (valueChangeByUser != 0) { startTime = millis(); }  // if a change was requested, also reset the timer
      bonkAfterDelay(customTimerDuration);                   // bonk if timer is up
      break;
    case 2:
      // random (aka "10 hours of silence occasionally broken up by klangschale")
      randomMax += valueChangeByUser;  // update max value in case a change was requested
      bonkAfterDelay(randomTimer);     // bonk if (random) timer is up
      break;
  }
}


/* TODOs:
- display current bpm/timer length to user (for timer you could use Julius method/just use a led or 7 segment or smth)
 */