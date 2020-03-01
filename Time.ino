void AffTime()
{
#if defined RTCMode && debugOn
  DateTime now = RTC.now();
  PrintSerial(now.year(), DEC);
  PrintSerial('/');
  PrintSerial(now.month(), DEC);
  PrintSerial('/');
  PrintSerial(now.day(), DEC);
  PrintSerial(' ');
  PrintSerial(now.hour(), DEC);
  PrintSerial(':');
  PrintSerial(now.minute(), DEC);
  PrintSerial(':');
  PrintSerialln(now.second(), DEC);
#endif
#if defined TimeMode && debugOn
  PrintSerialln("");
  PrintSerial(hour());
  PrintSerial(":");
  PrintSerial(minute());
  PrintSerial(":");
  PrintSerial(second());
  PrintSerial(" ");
  PrintSerial(day());
  PrintSerial("/");
  PrintSerial(month());
  PrintSerial("/");
  PrintSerial(year());
  PrintSerial(" ");
  PrintSerialln(weekday());
#endif
}
