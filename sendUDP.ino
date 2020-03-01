#define requestFrame true
#define responseFrame false


void SendRegisters()
{
  GatewayLink.PendingDataReqSerial[3] = registersResponse;
  uint8_t framLen = 4;
  for (int i = 0; i < registerSize; i++) {
    GatewayLink.PendingDataReqSerial[4 + 2 * i] = uint8_t(i);  // register number
    GatewayLink.PendingDataReqSerial[5 + 2 * i] = Registers[i]; // register value
    framLen = framLen + 2;
    if (framLen > sizeReqSerial - 2) {       // test not over maximal frem size
      break;
    }
  }
  FormatFrame(responseFrame, toAckFrame, framLen);
}
void SendIndicators()
{
  GatewayLink.PendingDataReqSerial[3] = indicatorsRequest;
  GatewayLink.PendingDataReqSerial[4] = 0x01; // first byte indicator position 1
  GatewayLink.PendingDataReqSerial[5] = 0x00; // padding
  int idx = 5;
  int value = random(-2000, 2000);           // 2 byte int second indicator is randomly choosen
#if defined(debugOn)
  PrintSerialln(value);
#endif
  byte *PretByte = FormatIntBytes(indicatorsRequest, value); // format data according to the command type
  for (int i = 1; i <= *PretByte; i++) {
    GatewayLink.PendingDataReqSerial[i + idx] = *(PretByte + i);  //  data tot the frame
  }
  idx = idx + *PretByte;
  GatewayLink.PendingDataReqSerial[idx + 1] = 0x00; //
  GatewayLink.PendingDataReqSerial[idx + 2] = uint8_t(indicator6); // 

  FormatFrame(responseFrame, toAckFrame, idx + 3);
}
void SendStatus(boolean ack)
{
  GatewayLink.PendingDataReqSerial[3] = statusResponse;
  GatewayLink.PendingDataReqSerial[4] = diagByte;
  GatewayLink.PendingDataReqSerial[5] = 0x00;
  GatewayLink.PendingDataReqSerial[6] = ver;
  GatewayLink.PendingDataReqSerial[7] = 0x00;
  int minuteSinceReboot = (millis() / 60000);
  GatewayLink.PendingDataReqSerial[8] = uint8_t(minuteSinceReboot / 256);
  GatewayLink.PendingDataReqSerial[9] = uint8_t(minuteSinceReboot);
  FormatFrame(responseFrame, ack, 0x0a);
}

void SendToGoogleSheet(uint8_t number, int values[], boolean ack)
{
  uint8_t idx = 3;
  number = min(number, maxValuesToSend);
  GatewayLink.PendingDataReqSerial[3] = insertDataInSheetRequest; // add command to the frame
  for (int j = 0; j < number; j++)
  {
    byte *PretByte = FormatIntBytes(insertDataInSheetRequest, values[j]); // format data according to the command type
    for (int i = 1; i <= *PretByte; i++) {
      GatewayLink.PendingDataReqSerial[i + idx] = *(PretByte + i);  //  data tot the frame
    }
    idx = idx + *PretByte;
  }
  FormatFrame(responseFrame, ack, min(uint8_t(idx + 1), sizeReqSerial));  // complete the frame format
}
void SendToDatabase(uint8_t number, uint8_t type, int values[], boolean ack)
{
  uint8_t idx = 4;
  number = min(number, maxValuesToSend);
  GatewayLink.PendingDataReqSerial[3] = insertDataInDatabaseRequest; // add command to the frame
  GatewayLink.PendingDataReqSerial[4] = type; // mesrument type
  for (int j = 0; j < number; j++)
  {
    byte *PretByte = FormatIntBytes(insertDataInDatabaseRequest, values[j]); // format data according to the command type
    for (int i = 1; i <= *PretByte; i++) {
      GatewayLink.PendingDataReqSerial[i + idx ] = *(PretByte + i); //  data tot the frame
    }
    idx = idx + *PretByte;
  }
  FormatFrame(responseFrame, ack, min(uint8_t(idx + 1), sizeReqSerial));  // complete the frame format
}
void SendTimeRequest()
{
  /*
     ask to the server for uptodate time
  */
  GatewayLink.PendingDataReqSerial[3] = timeUpdateRequest;
  FormatFrame(requestFrame, noAckFrame, 0x04);
}

void FormatFrame(boolean request, boolean toAck, uint8_t frameLen)
{
  /*
     format frame according to server protocol
  */
  trameNumber++;
  frameLen = min(frameLen, sizeReqSerial);
  frameLen = max(frameLen, minReqSerial);
  GatewayLink.PendingDataLenSerial = frameLen + 2;
  GatewayLink.frameSwitch = uint8_t(trameNumber % 256);; //
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  if (request) {
    bitWrite(GatewayLink.PendingReqSerial, requestFrameBit, 1);
  }
  else {
    bitWrite(GatewayLink.PendingReqSerial, requestFrameBit, 0);
  }
  if (toAck) {
    bitWrite(GatewayLink.PendingReqSerial, toAcknoledgeBit, 1);
  }
  else {
    bitWrite(GatewayLink.PendingReqSerial, toAcknoledgeBit, 0);
  }
  GatewayLink.PendingDataReqSerial[0] = unitGroup;
  GatewayLink.PendingDataReqSerial[1] = unitId;
  GatewayLink.PendingDataReqSerial[2] = 0x00;
  /*
     add 0x00 and CRC8 at the end of the frame
  */
  GatewayLink.PendingDataReqSerial[frameLen] = 0x00;
  GatewayLink.PendingDataReqSerial[frameLen + 1] = CRC8(GatewayLink.PendingDataReqSerial , frameLen);
  lastFormatedTime=millis();
#if defined(debugOn)
  // Serial.print("send CRC:0x");
  // Serial.println(CRC8(GatewayLink.PendingDataReqSerial , frameLen), HEX);
#endif
}
void SendAckFrame()
{
  GatewayLink.frameSwitch = uint8_t(trameNumber % 256);; //
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  GatewayLink.PendingDataReqSerial[0] = unitGroup;
  GatewayLink.PendingDataReqSerial[1] = unitId;
  GatewayLink.PendingDataReqSerial[3] = GatewayLink.DataInSerial[firstDataBytePosition];
  GatewayLink.PendingDataReqSerial[4] = 0x00;
  GatewayLink.PendingDataReqSerial[5] = 0x00;
  GatewayLink.PendingDataReqSerial[6] = CRC8(GatewayLink.PendingDataReqSerial , 5);
  GatewayLink.PendingDataLenSerial = 0x07;
#if defined(debugConnection)
  PrintSerial("format CRC:0x");
  PrintSerialln(CRC8(GatewayLink.PendingDataReqSerial , 5));
#endif
}
byte * FormatIntBytes(uint8_t type, int value)
{
#define retLen 4
  static uint8_t retByte[retLen];
  switch (type)
  {
    case indicatorsRequest:
      {
        {
          /*
             in this case data format is a sequence of  0xcdef signed 2 bytes int
          */
          retByte[0] = retLen - 2;
          if (value < 0)
          {
            value = -value;
            retByte[1] = uint8_t(((value & 0x7f00) >> 8) | 0x80); // add negative bit
          }
          else
          {
            retByte[1] = uint8_t(value  >> 8); // add negative bit
          }
          retByte[2] = uint8_t(value & 0x00ff );
          break;
        }
      }
    case insertDataInDatabaseRequest:
    case insertDataInSheetRequest:
      {
        /*
           in this case data format is a sequence of 0xab (sign character of the value) 0xcdef (non signed 2 bytes int)
        */
        retByte[0] = retLen - 1;
        if (value < 0)
        {
          retByte[1] = 0x2d;
        }
        else
        {
          retByte[1] = 0x2b;
        }
        retByte[2] = uint8_t((value & 0xff00) >> 8);
        retByte[3] = uint8_t(value & 0x00ff );
        break;
      }
  }
  return retByte;
}
