uint8_t ReadByte(int address)
{
#if defined(debugEeprom)
  PrintSerial("eeprom ");
  PrintSerial(address);
  PrintSerial(":");
  PrintSerialln(EEPROM.read(address), HEX);
#endif
  return EEPROM.read(address);
}
uint8_t WriteByte(int address, uint8_t value)
{
#if defined(debugEeprom)
  PrintSerial("update eeprom ");
  PrintSerial(address);
  PrintSerial(":");
  PrintSerialln(value, HEX);
#endif
  EEPROM.update(address, value);
  delay(5);
}
void LoadParameters()
{
  if ( EEPROM.read(eepromVersionAddr) != eepromVersion)
  {
#if defined(debugEeprom)
    PrintSerial("invalid eeprom version:0x");
    PrintSerialln(EEPROM.read(eepromVersionAddr), HEX);
    PrintSerialln("default coded values will be used");
#endif
    PrintEeprom();
    return;
  }
#if defined(debugEeprom)
  PrintSerial("unitGroup:0x");
  PrintSerial(EEPROM.read(eepromUnitIdAddr), HEX);
  PrintSerial(" unitId:0x");
  PrintSerialln(EEPROM.read(eepromUnitIdAddr + 1), HEX);
#endif
  bitWrite(diagByte, diagEeprom, 0);
  unitGroup = EEPROM.read(eepromUnitIdAddr);
  unitId = EEPROM.read(eepromUnitIdAddr + 1);
  for (uint8_t i = 0; i < registerSize; i++)
  {
    Registers[i] = EEPROM.read(eepromRegistersAddr + i);
#if defined(debugEeprom)
    PrintSerial(" reg:");
    SPrintSerial(Registers[i]);
#endif
  }
#if defined(debugEeprom)
  PrintSerialln();
#endif
}

void InitConfiguration()
{
  /*
     must be run once during the installation
  */
#if not defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  Serial.begin(38400);            // use serial for for any device during initial configuration
  delay(200);
  Serial.println("start serial1 for config ");
#endif
  Serial.println("init eeprom... wait... ");
  WriteByte(eepromVersionAddr, eepromVersion);
  delay(100);
  WriteByte(eepromUnitIdAddr, unitGroup);
  delay(100);
  WriteByte(eepromUnitIdAddr + 1, unitId);
  delay(100);
  for (int i = 0; i < registerSize; i++)
  {
    WriteByte(eepromRegistersAddr + i, Registers[i]);
    delay(100);
  }
  Serial.print("end init eeprom version:");
  Serial.println(eepromVersion);
  Serial.print("station group:");
  Serial.print(unitGroup, HEX);
  Serial.print(" station Id:");
  Serial.println(unitId, HEX);
  Serial.println("set config PIN off and reboot");
  delay(1000);
}

void PrintEeprom()
{
  for (int i = 0; i < 300; i++) {
    ReadByte(i);
  }
}

void SaveRegister(int  regId, uint8_t value) { 
  if (regId < registerSize && regId >= 0) {
    WriteByte(eepromRegistersAddr + regId, value);
  }
}
void SaveRegisters() {
  for (int i=0;i<registerSize;i++){
    SaveRegister(i, Registers[i]);
  }
}
