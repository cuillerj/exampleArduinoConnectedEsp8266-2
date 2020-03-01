#define frameNumberBytePosition 0
#define requestResponseBytePosition 2
#define lenBytePosition 4
#define commandBytePosition 7
#define firstDataBytePosition 8
#define ackByte 0x3f
#define cmdAckBitFlag 7
#define crcLen 2

//void TraitInput(uint8_t cmdInput) {
void TraitInput() {
  if (GatewayLink.DataInSerial[frameNumberBytePosition + 1] >= numberFrameSource) {
#if defined(debugConnection)
    PrintSerial("unknown frame source:");
    PrintSerialln(GatewayLink.DataInSerial[frameNumberBytePosition + 1]);
    return;
#endif
  }
  lastReceivedTime = millis();
  bitWrite(diagByte, diagServerConnexion, 0);
  if (GatewayLink.DataInSerial[requestResponseBytePosition] == ackByte)   // is it an aknoledgment ?
  {
    lastAckTrameNumber[GatewayLink.DataInSerial[frameNumberBytePosition + 1]] = GatewayLink.DataInSerial[frameNumberBytePosition];
#if defined(debugConnection)
    PrintSerial("ack:");
    PrintSerialln(lastAckTrameNumber[GatewayLink.DataInSerial[frameNumberBytePosition + 1]]);
#endif
    if (lastAckTrameNumber[GatewayLink.DataInSerial[frameNumberBytePosition + 1]] == trameNumber)
    {
      pendingAckSerial = 0x00;
      bitWrite(diagByte, diagServerConnexion, 0);
#if defined(debugConnection)
      PrintSerialln("no ack pending");
#endif
    }
    return;
  }
  uint8_t inputCRC = GatewayLink.DataInSerial[GatewayLink.DataInSerial[lenBytePosition] - 1];
  uint8_t computeCRC = CRC8(&GatewayLink.DataInSerial[commandBytePosition] , GatewayLink.DataInSerial[lenBytePosition] - commandBytePosition - crcLen);
#if defined(debugConnection)
  PrintSerialln("receive ");
  for (int i = 0; i <= min(GatewayLink.DataInSerial[lenBytePosition], 30); i++)
  {
    PrintSerial("0x");
    PrintSerial(GatewayLink.DataInSerial[i]);
    PrintSerial("-");
  }
  PrintSerialln("");
  PrintSerial("len:");
  PrintSerial(GatewayLink.DataInSerial[lenBytePosition]);
  PrintSerial(" inCrc:");
  PrintSerial(inputCRC);
  PrintSerial(" expCrc:0x");
  PrintSerialln(computeCRC);
#endif
  if (inputCRC != computeCRC)
  {
    bitWrite(diagByte, diagServerConnexion, 1);
#if defined(debugConnection)
    PrintSerialln("crcError");
#endif
    return;
  }
  else {
    bitWrite(diagByte, diagServerConnexion, 0);
  }

  if (bitRead(GatewayLink.DataInSerial[requestResponseBytePosition], cmdAckBitFlag)) // does this frame to be acknoledge ?
  {
    SendAckFrame();
  }
  if (!bitRead(GatewayLink.DataInSerial[requestResponseBytePosition], commandBytePosition)) // this is a response ?
  {
#if defined(debugConnection)
    PrintSerial("response: 0x");
    PrintSerialln(GatewayLink.DataInSerial[commandBytePosition]);
#endif
    switch (GatewayLink.DataInSerial[commandBytePosition]) {
      case timeUpdateResponse:
        {
          /*
            convert byte input to char date and time to setup clock
            input[0] id j number, [1] month number, [2] year number minus 2000, [3] hour number, [4]minute number, [5] second number
          */
#if defined(debugConnection)
          PrintSerial ("time response ");
          PrintSerialln(GatewayLink.DataInSerial[firstDataBytePosition + 1]);
#endif
          char DateToInit[15] = "xxx xx 20xx";
          char TimeToInit[9] = "xx: xx: xx";
          lastUpdateClock = millis();
          pendingRequest = 0x00;
          if (GatewayLink.DataInSerial[firstDataBytePosition + 1] > 0 && GatewayLink.DataInSerial[firstDataBytePosition + 1] < 13)
          {
            DateToInit[0] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1)];
            DateToInit[1] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1) + 1];
            DateToInit[2] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1) + 2];
            DateToInit[4] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition ] / 10 + 48);
            DateToInit[5] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition ] - (GatewayLink.DataInSerial[firstDataBytePosition ] / 10) * 10) + 48);
            DateToInit[9] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 3] / 10 + 48); //
            DateToInit[10] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 3] - (GatewayLink.DataInSerial[firstDataBytePosition + 3] / 10) * 10) + 48);
            TimeToInit[0] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 4] / 10 + 48);
            TimeToInit[1] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 4] - (GatewayLink.DataInSerial[firstDataBytePosition + 4] / 10) * 10) + 48);
            TimeToInit[3] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 6] / 10 + 48);
            TimeToInit[4] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 6] - (GatewayLink.DataInSerial[firstDataBytePosition + 6] / 10) * 10) + 48);
            TimeToInit[6] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 7] / 10 + 48);
            TimeToInit[7] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 7] - (GatewayLink.DataInSerial[firstDataBytePosition + 7] / 10) * 10) + 48);
            bitWrite(diagByte, diagTimeUpToDate, 0);

#if defined(debugConnection)
            PrintSerialln("setting time");
#endif
#if defined RTCMode
            RTC.adjust(DateTime(DateToInit, TimeToInit));
#endif
#if defined TimeMode
            int hh = GatewayLink.DataInSerial[firstDataBytePosition + 4];
            int mn = GatewayLink.DataInSerial[firstDataBytePosition + 6];
            int ss = GatewayLink.DataInSerial[firstDataBytePosition + 7];
            int jj = GatewayLink.DataInSerial[firstDataBytePosition];
            int mm = GatewayLink.DataInSerial[firstDataBytePosition + 1] ;
            int aa = 2000 + GatewayLink.DataInSerial[firstDataBytePosition + 3];
            setTime(hh, mn, ss, jj, mm,  aa);
#endif
#if defined(debugConnection)
            PrintSerial("set time ");
            AffTime();
#endif
          }
          break;
        }

    }
  }
  else  // this is a command
  {
#if defined(debugConnection)
    PrintSerial("request: 0x");
    PrintSerialln(GatewayLink.DataInSerial[commandBytePosition]);
#endif
    switch (GatewayLink.DataInSerial[commandBytePosition]) {

      case updateRegistersRequest:   // set register value
        {
#if defined(debugConnection)
          PrintSerialln("updateRegisterRequest");

#endif
          for (int i = 0; i < floor((GatewayLink.DataInSerial[lenBytePosition] - firstDataBytePosition - crcLen) / 2); i++) {
            if (GatewayLink.DataInSerial[firstDataBytePosition + 2 * i] >= 0 && GatewayLink.DataInSerial[firstDataBytePosition + 2 * i] < registerSize)
            {
              Registers[GatewayLink.DataInSerial[firstDataBytePosition + 2 * i]] = GatewayLink.DataInSerial[firstDataBytePosition + 2 * i + 1];
#if defined(debugConnection)
              PrintSerial("id ");
              PrintSerial(i);
              PrintSerial(" value: 0x");
              PrintSerialln(Registers[i]);
#endif
            }
          }

          timeSendRegister = millis() - 1000;
          //    SendRegisters();
          break;
        }

      case indicatorsRequest:   //
        {
#define stationFirstBytePosition firstDataBytePosition
#define indicatorFirstBytePosition firstDataBytePosition+3
#define valueFirstBytePosition firstDataBytePosition+6
          if (GatewayLink.DataInSerial[stationFirstBytePosition] != unitGroup || GatewayLink.DataInSerial[stationFirstBytePosition + 1] != unitId) {
#if defined(debugConnection)
            PrintSerial("wrong station address:");
            PrintSerial(GatewayLink.DataInSerial[stationFirstBytePosition]);
            PrintSerial("-");
            PrintSerialln(GatewayLink.DataInSerial[stationFirstBytePosition + 1]);
#endif
            return;
          }
          //       int indicator = GatewayLink.DataInSerial[indicatorFirstBytePosition] & 0x7f;
          //       indicator = indicator * 256 + GatewayLink.DataInSerial[indicatorFirstBytePosition + 1];
          int indicator = GetValue(GatewayLink.DataInSerial[indicatorFirstBytePosition], GatewayLink.DataInSerial[indicatorFirstBytePosition + 1]);

          //        int value = GatewayLink.DataInSerial[valueFirstBytePosition + 1];
          //        value = value * 256 + GatewayLink.DataInSerial[valueFirstBytePosition + 2];

          int value = GetValue(GatewayLink.DataInSerial[indicatorFirstBytePosition + 3], GatewayLink.DataInSerial[indicatorFirstBytePosition + 4]);
          if (GatewayLink.DataInSerial[valueFirstBytePosition + 2] == 0x2d) {
            value = -value;
          }
          
          if (GatewayLink.DataInSerial[indicatorFirstBytePosition] & 0x80) { // does server request indicator value

#if defined(debugConnection)
            PrintSerialln("indicators values are requested");
#endif
            timeSendIndicator = millis() - 1000;
            // SendIndicators();
          }
          else {   // indicator to be updated
#define Indicator6 6  // mapping with the id of the inddesc table
            switch (indicator) {
              case Indicator6:
                {
#if defined(debugOn)
                  PrintSerial("update indicator: ");
                  PrintSerial(indicator);
                  PrintSerial(" ");
                  PrintSerialln(value);
#endif
                  indicator6 = value;
                  timeSendIndicator = millis() - 1000;
                  //     SendIndicators();
                  break;
                }
              default:
                {
#if defined(debugConnection)
                  PrintSerialln("ignored");
#endif
                }
            }
          }
          break;
        }
      case writeEepromRequest:   // write eeprom
        {
#define saveRegistersBit 3
#if defined(debugConnection)
          PrintSerial("writeEepromRequest: param 0x");
          PrintSerialln(GatewayLink.DataInSerial[firstDataBytePosition]);
#endif
          if (bitRead(GatewayLink.DataInSerial[firstDataBytePosition], saveRegistersBit))
          {
#if defined(debugInput)
            PrintSerialln("writeEeprom registers");
#endif
            SaveRegisters();
          }
          break;
        }
    }
  }
}
int GetValue(byte one, byte two) {
  return ((one << 8) & 0x7fff) + (int(two) & 0x00ff);
}
unsigned int GetUnsignedValue(byte one, byte two) {
  return ((one << 8) ) + (int(two) & 0x00ff);
}
