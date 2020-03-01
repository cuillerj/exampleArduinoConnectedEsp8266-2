#if (defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__))
void PrintSerial(String txt) {
#if defined(serial1)
  Serial1.print(txt);
#endif
#if defined(serial2)
  Serial2.print(txt);
#endif
#if defined(serial3)
  Serial3.print(txt);
#endif
}
void PrintSerialln(String txt) {
#if defined(serial1)
  Serial1.println(txt);
#endif
#if defined(serial2)
  Serial2.println(txt);
#endif
#if defined(serial3)
  Serial3.println(txt);
#endif
}
void PrintSerial(int txt) {
#if defined(serial1)
  Serial1.print(txt);
#endif
#if defined(serial2)
  Serial2.print(txt);
#endif
#if defined(serial3)
  Serial3.print(txt);
#endif
}
void PrintSerialln(int txt) {
#if defined(serial1)
  Serial1.println(txt);
#endif
#if defined(serial2)
  Serial2.println(txt);
#endif
#if defined(serial3)
  Serial3.println(txt);
#endif
}
void PrintSerial(byte txt) {
#if defined(serial1)
  Serial1.print(txt, HEX);
#endif
#if defined(serial2)
  Serial2.print(txt, HEX);
#endif
#if defined(serial3)
  Serial3.print(txt, HEX);
#endif
}
void PrintSerialln(byte txt) {
#if defined(serial1)
  Serial1.println(txt, HEX);
#endif
#if defined(serial2)
  Serial2.println(txt, HEX);
#endif
#if defined(serial3)
  Serial3.println(txt, HEX);
#endif
}
void PrintSerial(unsigned long txt) {
#if defined(serial1)
  Serial1.print(txt);
#endif
#if defined(serial2)
  Serial2.print(txt);
#endif
#if defined(serial3)
  Serial3.print(txt);
#endif
}
void PrintSerialln(unsigned long txt) {
#if defined(serial1)
  Serial1.print(txt);
#endif
#if defined(serial2)
  Serial2.print(txt);
#endif
#if defined(serial3)
  Serial3.print(txt);
#endif
}
#endif
