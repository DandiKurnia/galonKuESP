#define FLOW_PIN 17
#define RELAY_PIN 20

volatile unsigned long pulseCount = 0;

// Kalibrasi YF-S201
// Umumnya YF-S201: 450 pulse = 1 liter
float pulsesPerLiter = 360.0;

float volumeLiter = 0.0;

// Ubah sesuai jenis relay kamu
// Banyak relay module aktif LOW:
// LOW = relay ON / tertutup
// HIGH = relay OFF / terbuka
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

void IRAM_ATTR countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, RELAY_OFF);

  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), countPulse, FALLING);

  Serial.println("YF-S201 + Relay Test Start");
}

void loop() {
  noInterrupts();
  unsigned long pulses = pulseCount;
  interrupts();

  volumeLiter = pulses / pulsesPerLiter;

  Serial.print("Pulse: ");
  Serial.print(pulses);
  Serial.print(" | Volume: ");
  Serial.print(volumeLiter, 3);
  Serial.println(" L");

  if (volumeLiter >= 0.5) {
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.println("Volume sudah 1 Liter -> Relay ON / tertutup");
    delay(50000);
  } else {
    digitalWrite(RELAY_PIN, RELAY_ON);
  }

  delay(200);
}