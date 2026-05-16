/*
  Turnout Control via Serial
  ---------------------------
  Commands are sent as: <NNX>
    NN = two-digit turnout number (00, 01, 02 ...)
     X = N (Normal) or R (Reverse)

  Examples:
    <00N>  →  pulse pin 8 LOW for 40 ms  (turnout 0, Normal)
    <00R>  →  pulse pin 9 LOW for 40 ms  (turnout 0, Reverse)
    <01N>  →  pulse pin 10 LOW for 40 ms (turnout 1, Normal)  [future]

  Pins are active-LOW: idle HIGH, pulsed LOW to throw the turnout.
*/

// ---------------------------------------------------------------------------
// Turnout table — add a row for every new turnout
// ---------------------------------------------------------------------------
struct Turnout {
  uint8_t normalPin;   // pin pulsed for Normal  (N)
  uint8_t reversePin;  // pin pulsed for Reverse (R)
};

const Turnout TURNOUTS[] = {
  {8, 9},   // turnout 00
  // {10, 11}, // turnout 01  ← uncomment and extend as needed
};

const uint8_t NUM_TURNOUTS = sizeof(TURNOUTS) / sizeof(TURNOUTS[0]);
const uint16_t PULSE_MS    = 40;   // pulse duration in milliseconds

// ---------------------------------------------------------------------------
// Serial command parser
// ---------------------------------------------------------------------------
const uint8_t  CMD_LEN     = 3;    // digits + direction inside < >
char           cmdBuffer[256];
uint8_t        cmdIndex    = 0;
bool           inCommand   = false;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Initialise all turnout pins HIGH (idle / de-energised)
  for (uint8_t i = 0; i < NUM_TURNOUTS; i++) {
    pinMode(TURNOUTS[i].normalPin,  OUTPUT);
    pinMode(TURNOUTS[i].reversePin, OUTPUT);
    digitalWrite(TURNOUTS[i].normalPin,  HIGH);
    digitalWrite(TURNOUTS[i].reversePin, HIGH);
  }

  Serial.println(F("### M0ZJO Turnout controller ready! ###"));
  Serial.println(F("Send <##R> or <##N>  e.g. <00N> <00R>"));
}

// ---------------------------------------------------------------------------
void loop() {
  // Read every available byte and build a command when < ... > is detected
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '<') {
      // Start of a new command — reset buffer
      inCommand = true;
      cmdIndex  = 0;
      memset(cmdBuffer, 0, sizeof(cmdBuffer));

    } else if (c == '>' && inCommand) {
      // End of command — process it
      inCommand = false;
      if (cmdIndex == CMD_LEN) {
        processCommand(cmdBuffer);
      } else {
        Serial.println(F("ERR: bad command length"));
      }

    } else if (inCommand) {
      // Accumulate characters, guard against overflow
      if (cmdIndex < CMD_LEN) {
        cmdBuffer[cmdIndex++] = c;
      } else {
        // Too many characters before '>' — discard
        inCommand = false;
        Serial.println(F("ERR: command too long"));
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Parse "NNX" and fire the appropriate pin
//   cmdBuffer = e.g. "00N"  "01R"
// ---------------------------------------------------------------------------
void processCommand(const char* cmd) {
  // Validate: first two chars are digits, third is N or R
  if (!isDigit(cmd[0]) || !isDigit(cmd[1])) {
    Serial.println(F("ERR: turnout number must be two digits"));
    return;
  }

  uint8_t id = (cmd[0] - '0') * 10 + (cmd[1] - '0');
  char    dir = cmd[2];   // 'N' or 'R'

  if (id >= NUM_TURNOUTS) {
    Serial.print(F("ERR: unknown turnout "));
    Serial.println(id);
    return;
  }

  if (dir != 'N' && dir != 'R') {
    Serial.println(F("ERR: direction must be N or R"));
    return;
  }

  uint8_t pin = (dir == 'N') ? TURNOUTS[id].normalPin : TURNOUTS[id].reversePin;

  // Pulse the pin
  digitalWrite(pin, LOW);
  delay(PULSE_MS);
  digitalWrite(pin, HIGH);

  // Confirm over Serial
  Serial.print(F("OK: turnout "));
  Serial.print(id);
  Serial.print(F(" -> "));
  Serial.println((dir == 'N') ? F("Normal") : F("Reverse"));
}
