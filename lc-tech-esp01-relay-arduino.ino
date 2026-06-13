/*
  LC Technology ESP-01 Relay v2.3 minimal Arduino example.

  This relay board does not connect the relay directly to an ESP8266 GPIO.
  The relay is controlled by the board's own MCU. The ESP-01 normally runs
  Espressif AT firmware, receives TCP data over Wi-Fi, and reports that data
  to the board MCU over UART as +IPD messages.

  This sketch does not use Wi-Fi. It only sends the UART messages that the
  board MCU expects to see from AT firmware, then embeds the relay command
  bytes inside those messages.

  Board setup:
  - Power the relay board from the correct relay voltage.
    For SRD-05VDC-SL-C relay boards, use 5V on IN+ / IN-.
  - Install the TX/RX jumper caps in the normal ESP-01 position.
  - Flash the ESP-01, then remove GPIO0 from GND and reset/power-cycle.

  Relay commands from the vendor documentation:
  - ON/open:  A0 01 01 A2
  - OFF/close: A0 01 00 A1
*/

const byte RELAY_ON[] = {0xA0, 0x01, 0x01, 0xA2};
const byte RELAY_OFF[] = {0xA0, 0x01, 0x00, 0xA1};

void blink(byte count) {
  pinMode(LED_BUILTIN, OUTPUT);

  for (byte i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
  }
}

void sendLine(const char *line) {
  Serial.print("\r\n");
  Serial.print(line);
  Serial.print("\r\n");
  Serial.flush();
}

void sendAtReadyState() {
  // Some board modes wait until AT firmware reports a successful Wi-Fi state.
  // These text events are enough for this relay MCU; no real Wi-Fi is needed.
  sendLine("ready");
  delay(100);
  sendLine("WIFI CONNECTED");
  delay(100);
  sendLine("WIFI GOT IP");
  delay(100);
}

void sendIpdFrame(const byte *payload, size_t length) {
  // Single-connection ESP-AT format: +IPD,<length>:<payload>
  Serial.print("+IPD,");
  Serial.print(length);
  Serial.print(":");
  Serial.write(payload, length);
  Serial.flush();
}

void sendMultiIpdFrame(byte connectionId, const byte *payload, size_t length) {
  // Multi-connection ESP-AT format: +IPD,<id>,<length>:<payload>
  Serial.print("+IPD,");
  Serial.print(connectionId);
  Serial.print(",");
  Serial.print(length);
  Serial.print(":");
  Serial.write(payload, length);
  Serial.flush();
}

void sendRelayCommand(const byte *command, size_t length) {
  // Mimic the AT firmware messages produced when a TCP client connects and
  // sends four relay-control bytes. Sending both +IPD forms covers both board
  // modes observed on LC Technology v2.3 relay boards.
  sendLine("0,CONNECT");
  delay(100);

  sendIpdFrame(command, length);
  delay(100);
  sendMultiIpdFrame(0, command, length);
  delay(100);

  sendLine("0,CLOSED");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  sendAtReadyState();
  blink(3);
}

void loop() {
  blink(1);
  sendAtReadyState();
  sendRelayCommand(RELAY_ON, sizeof(RELAY_ON));
  delay(2000);

  blink(2);
  sendAtReadyState();
  sendRelayCommand(RELAY_OFF, sizeof(RELAY_OFF));
  delay(5000);
}
