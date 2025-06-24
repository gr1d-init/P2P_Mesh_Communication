#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define BTN_GREEN 2
#define BTN_YELLOW 3
#define BTN_RED 4 

#define LED_GREEN 5
#define LED_YELLOW 6
#define LED_RED 7

RF24 radio(9, 10); // CE, CSN

// Update addresses for each node
const byte thisNode[6] = "NODE2";
const byte otherNode[6] = "NODE1";

// Store button and LED states
bool buttonStates[3] = {0, 0, 0};
bool lastButtonStates[3] = {0, 0, 0};
bool ledStates[3] = {0, 0, 0};

void setup() {
  Serial.begin(9600);

  pinMode(BTN_GREEN, INPUT_PULLUP);
  pinMode(BTN_YELLOW, INPUT_PULLUP);
  pinMode(BTN_RED, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, HIGH);

  radio.begin();
  radio.openWritingPipe(otherNode);  // Address to send to
  radio.openReadingPipe(1, thisNode); // Address to receive on
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();  // Start in transmit mode
}

void loop() {
  // READ BUTTONS
  buttonStates[0] = !digitalRead(BTN_GREEN);  // Active LOW
  buttonStates[1] = !digitalRead(BTN_YELLOW);
  buttonStates[2] = !digitalRead(BTN_RED);

  // IF BUTTON CHANGED → Send
  bool sendNeeded = false;
  for (int i = 0; i < 3; i++) {
    if (buttonStates[i] != lastButtonStates[i]) {
      sendNeeded = true;
      lastButtonStates[i] = buttonStates[i];
    }
  }

  if (sendNeeded) {
    radio.stopListening();  // Switch to TX mode
    radio.write(&buttonStates, sizeof(buttonStates));
  }

  // RECEIVE if data available
  radio.startListening();
  unsigned long startTime = millis();
  while (!radio.available()) {
    if (millis() - startTime > 5) break;  // Non-blocking wait
  }

  if (radio.available()) {
    radio.read(&ledStates, sizeof(ledStates));
    digitalWrite(LED_GREEN, !ledStates[0]);
    digitalWrite(LED_YELLOW, !ledStates[1]);
    digitalWrite(LED_RED, !ledStates[2]);
  }

  delay(10);  // Small delay to avoid flooding
}
