
/* Detects patterns of knocks and triggers a servo to open a latch
   for Secret Knock Box project. See tutorial at http://fabforgefive.com/secret-knock

  Contains code from Secret Knock Gumball Machine Version 10.12.20 written by
  Steve Hoefer http://grathio.com, used with permission. Thanks Steve!
  
  Licensed under Creative Commons Attribution-Noncommercial-Share Alike 3.0
  http://creativecommons.org/licenses/by-nc-sa/3.0/us/
  (Don't relicense it and don't sell it or use it in anything you sell without 
  contacting me first. Greg Treseder, greg@fabforgefive.com. I do like to share and 
  would love to hear from you.)

  Analog Pin 0: Piezo speaker (connected to ground with 470K ohm pulldown resistor)
  Digital Pin 10: Switch to enter new knock code.
  Digital Pin 13: LED.
  Digital Pin 11: Servo motor signal.
  Digital Pin 12: buzzer

  This fabforgefive version adds EEPROM write/read to save custom knocks after power-off
  But kinda dangerous if batteries fail! So make sure to have a secret emergency manual unlatch!
*/

#include <Servo.h>
#include <EEPROM.h>

// Pin definitions
const int knockSensor = 0;         // Piezo sensor on analog pin 0.
const int trimPot = 1;             // Trim potentiometer that allows the user to set the sensitivity
const int programSwitch = 10;       // When this is high we program a new code
const int servoPin = 11;            // Servo that we're controlling.
const int vibrate = 12;              // buzzer
const int LED = 13;              // Status LED for failed knock

// the current address in the EEPROM (i.e. which byte we're going to write to next)
int addr = 0;

// Tuning constants.  Tested for use with 27mm Piezo disc and 474K ohm resistor.
const int thresholdMax = 40;       // Maximum value we can set for the knock sensitivity based on the trim pot.  Increase this if it's too sensitive.
const int rejectValue = 30;        // If an individual knock is off by this percentage of a knock we ignore. (30 is pretty lose. 10 is strict)
const int averageRejectValue = 20; // If the average timing of the knocks is off by this percent we ignore. (20 is pretty lose, 10 is strict.)
const int debounceThreshold = 150;  // Simple debounce timer to make sure we don't register more than one knock.
const int maximumKnocks = 20;       // Maximum number of knocks to listen for.
const int knockComplete = 1500;     // Longest time to wait for a knock before we assume that it's finished.


// Variables.
Servo latchServo;                  // The servo that opens the latch
int secretCode[maximumKnocks];  //array for secret code
int unlock = 130; //move servo to this position to unlock
int knockReadings[maximumKnocks];    // When someone knocks this array fills with delays between knocks. (A correct knock looks a lot like the line above).
int knockSensorValue = 0;            // Most recent reading of the knock sensor.
int threshold = 50;                // Minimum signal from the piezo to register as a knock.  (Set by the trim pot.)
int programButtonPressed = false;    // Flag so we remember the programming button setting at the end of the cycle.
long counter = 0;                    // Used to time the display light and the trim pot readings.

void setup() {

  pinMode(vibrate, OUTPUT);

  pinMode(programSwitch, INPUT);
  digitalWrite(programSwitch, HIGH);   // turn on pullup resistors


  latchServo.attach(servoPin);   // Initialize the servo.
  delay(50);
  latchServo.write(0);
  delay(500);  // wait for servo to get there
  latchServo.detach();   // Detach the servo when not using it.

  //buzz to signal we're ready
  digitalWrite(vibrate, HIGH);    //vibrate
  delay(150);
  digitalWrite(vibrate, LOW);
  delay(150);
  digitalWrite(vibrate, HIGH);    //vibrate
  delay(150);
  digitalWrite(vibrate, LOW);

  for (int i = 0; i < maximumKnocks; i++) { // Read the secret knock from EEPROM
    secretCode[i] = EEPROM.read(i);
  }
}

void loop() {
  // Listen for any knock at all.
  knockSensorValue = analogRead(knockSensor);
  counter++;
  if (counter >= 1500) {
    counter = 0;
  }


  if (digitalRead(programSwitch) == LOW) { // Is the program button pressed?
    programButtonPressed = true;           // Yes, so lets save that state.

    digitalWrite(LED, HIGH);    //turn on the the light so we know we're programming.
  }
  else {
    programButtonPressed = false;          // Otherwise reset all the ... everything.
    digitalWrite(LED, LOW);

  }
  if (knockSensorValue > threshold) {
    listenToSecretKnock();
  }
}

// Records the timing of knocks.
void listenToSecretKnock() {
  //Serial.println("(knock started)");

  int i = 0;
  int counter = 0;
  // First lets reset the listening array.
  for (i = 0; i < maximumKnocks; i++) {
    knockReadings[i] = 0;
  }

  int currentKnockNumber = 0;     // Increment for the array.
  int startTime = millis();       // Reference for when this knock started.
  int now = millis();

  digitalWrite(LED, HIGH);   // Light the LED for knock feedback.

  delay(debounceThreshold);
  digitalWrite(LED, LOW);

  do {
    // Listen for the next knock or wait for it to time out.
    knockSensorValue = analogRead(knockSensor);

    if (knockSensorValue > threshold) {                  // Got another knock...
      now = millis();                                    // Record the delay time.
      knockReadings[currentKnockNumber] = now - startTime;
      currentKnockNumber ++;                             // Increment the counter.
      startTime = now;
      digitalWrite(LED, HIGH);      	         // Light the knock LED.

      delay(debounceThreshold / 2);                      // Debounce the knock sensor. (And turn the LED off halfway through.)
      digitalWrite(LED, LOW);
      delay(debounceThreshold / 2);
    }

    now = millis();

    // Did we timeout or have too many knocks?
  }
  while ((now - startTime < knockComplete) && (currentKnockNumber < maximumKnocks));

  // We have a completed knock, lets see if it's valid
  if (programButtonPressed == false) {
    if (validateKnock() == true) {
      triggerSuccessfulAction();
    }
    else {
      triggerFailedAction();
    }
  }
  else { // If we're in programming mode we still run it through the validate function in order to get some useful numbers.
    validateKnock();


    digitalWrite(LED, LOW);
    for (i = 0; i < 2; i++) {
      delay(100);
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
    }
  }
}

// We got a good knock, so do something!
void triggerSuccessfulAction() {
  int i = 0;

  digitalWrite(LED, HIGH);       // Turn on the LED so the user knows the knock was successful.  (Important if the optional status lights are missing.)

  latchServo.attach(servoPin);    // init the servo.
  latchServo.write(0);
  delay(100);
  latchServo.write(unlock); //release the latch
  delay(1000);

  latchServo.write(0); //return to be ready to latch again
  delay(500);

  latchServo.detach();

}

// We didn't like the knock.  Indicate displeasure.
void triggerFailedAction() {
  digitalWrite(vibrate, HIGH);
  delay(1000);
  digitalWrite(vibrate, LOW);
  delay(100);
}

// Checks if our knock matches the secret.
// Returns true if it's a good knock, false if it's not.
boolean validateKnock() {
  int i = 0;

  // Simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;             // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode[i] > 0) {
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) { 	// Collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info to EEPROM.

  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) {
      secretCode[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100); // Normalize the knock timing
      EEPROM.write(i, secretCode[i]);    //save new knock to EEPROM

    }
    // And flash the lights in the recorded pattern to let us know it's been programmed.
    digitalWrite(LED, LOW);
    delay(750);

    //Start playing back the knocks
    digitalWrite(LED, HIGH);  // First knock
    delay(40);
    for (i = 0; i < maximumKnocks ; i++) {
      digitalWrite(LED, LOW);

      if (programButtonPressed == true) { // Only turn it on if there's a delay
        if (secretCode[i] > 0) {
          delay(map(secretCode[i], 0, 100, 0, maxKnockInterval)); // Expand the time back out to what it was.  Roughly.
          digitalWrite(LED, HIGH);
        }
      }
      delay(40);
      digitalWrite(LED, LOW);
    }
    return false; 	// We don't do anything when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;   // Return false if the number of knocks are wrong.
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
    (ie: if you do the same pattern slow or fast it should still work.)
    This makes it less picky, which does make it less secure but also makes it
    less of a pain to use if you're tempo is a little slow or fast.
  */
  boolean codeFound = true;
  int totaltimeDifferences = 0;
  int timeDiff = 0;

  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      codeFound = false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    codeFound = false;
  }

  if (codeFound == false) {
    return false;
  }
  else {
    return true;
  }

}
