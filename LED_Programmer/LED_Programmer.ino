
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setAddress(int address, bool  outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address >> 8 | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);

}

byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }

  setAddress(address, true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1 ) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

void writeEEPROM(int address, byte data) {
  setAddress(address, false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

void printContents() {
  for (int base = 0; base <= 1023; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1 ) {
      data[offset] = readEEPROM(base + offset);
    }
    char buf[80];
    sprintf(buf, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
}


// Erase entire EEPROM
void eraseEEPROM() {
  Serial.print("Erasing EEPROM");
  for (int address = 0; address <= 2047; address += 1) {
    writeEEPROM(address, 0xff);
    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println();
}

// 4-bit hex decoder for common cathode 7-segment display
// byte data[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47 };

void setup() {
  // put your setup code here, to run once:

  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  //eraseEEPROM();

  byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };

  Serial.print("Programming EEPROM...");

  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value, digits[value % 10]) ;
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
    writeEEPROM(value + 768, 0);
  }

  Serial.println(" done");

  printContents();

}



void loop() {
  // put your main code here, to run repeatedly:
}
