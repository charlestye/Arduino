
// Modification of Ben Eater's Display EEPROM program to show opcodes in 0-15 of the alternative character set.
// Tie A10 (pin 19) high on the 28C16 EEPROM on the CPU.
//
// Sample code to display. Adjust the clock speed low.
//
// 0:   LDI     0
// 1:   OUT
// 2:   ADD     4
// 3:   JMP     1
// 4:   1



#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setAddress(int address, bool outputEnable) {
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

  // eraseEEPROM();

  // List of Opcodes
  String opcodes[16] = {"DSP ", "LDA ", "ADD ", "SUB ", "STA ", "LDI ", "JMP ", "JC  ", "JZ  ", "JNC ", "CMP ", "INC ", "DEC ", "OUT ", "HLT "};

  byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };
  byte letters[] = { 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47, 0x7b, 0x37, 0x30, 0x3c, 0x37, 0x0e, 0x55, 0x15, 0x7e, 0x67, 0x73, 0x05, 0x5b, 0x0f, 0x3e, 0x27, 0x3f, 0x25, 0x3b, 0x6d };

  Serial.print("Programming EEPROM...");

  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value, digits[value % 10]) ;
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
    writeEEPROM(value + 768, 0);
  }

  //write opcodes into top 1024 bytes of EEPROM

  for (int value = 0; value <= 15; value += 1) {

    byte opbytes[4];
    byte ledbytes[4];
    opcodes[value].getBytes(opbytes, 4);

    for (int i = 0; i <= 3; i += 1) {
      byte b = opbytes[i];
      if (b >= 48 && b <= 57) {
        ledbytes[i] = digits[b - 48]; //digit
      }
      else if (b >= 65 && b <= 90) {
        ledbytes[i] = letters[b - 65]; //letter
      }
      else {
        ledbytes[i] = 0 ; //everything else is a space
      }

      writeEEPROM(value + 1024, ledbytes[3] );
      writeEEPROM(value + 1024 + 256, ledbytes[2] );
      writeEEPROM(value + 1024 + 512, ledbytes[1] );
      writeEEPROM(value + 1024 + 768, ledbytes[0] );
    }

    Serial.println(" done");
  }
  
  printContents();

}

  void loop() {
    // put your main code here, to run repeatedly:
  }
