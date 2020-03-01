void StandardLoop()
{
#if defined GatewayReadyPIN
  if (digitalRead(GatewayReadyPIN)) {
    digitalWrite(gatewayReadyLED, 1);
  }
  else {
    digitalWrite(gatewayReadyLED, 0);
  }
#endif

  /*
         send and receive loop based on timers
  */
  if (GatewayLink.SerialActive()) {

    // ***  keep in touch with the server
    int getSerial = GatewayLink.Serial_have_message();  // check if we have received a message
    if (getSerial > 0)                                  // we got a message
    {
#if defined(debugConnection)
      PrintSerialln("input serial");
#endif
      //     TraitInput(GatewayLink.DataInSerial[receiveFlagPosition]);          // analyze the message
      TraitInput();          // analyze the message
    }
    if (GatewayLink.PendingReqSerial != 0x00)           // check if we have a message to send (or a message currently sending)
    {
      GatewayLink.DataToSendSerial();                    // send message on the serial link
      timeSendSecSerial = millis();                      // reset timer
    }
    if ( millis() - timeSendSecSerial >= 5000 && pendingAckSerial != 0x00 && retryCount < 5) {
      GatewayLink.ReSendSecuSerial();                    // re-send data that was not already ack by the server
#if defined(debugOn)
     PrintSerialln("retry");
#endif
      timeSendSecSerial = millis();         // clear timer
      retryCount = retryCount + 1;          // update rerty count
    }
    if (retryCount >= 5)
    {
      retryCount = 0;
      pendingAckSerial = 0x00;
    }
    /*
       below enter your specific code
    */

#if defined TimeOn
    if (GatewayLink.PendingReqSerial == 0x00 && bitRead(diagByte, diagTimeUpToDate) && (millis() - pendingRequestClock > 30000))
    {
      SendTimeRequest();
      pendingRequest = timeUpdateRequest;
      pendingRequestClock = millis();
    }
#endif
    if ((millis() - lastUpdateClock > updateClockLimit) && (millis() - pendingRequestClock > 30000))
    {
      bitWrite(diagByte, diagTimeUpToDate, 1);
      pendingRequestClock = millis();
      //        lastUpdateClock = millis();
    }
    if (millis() - pendingRequestClock > pendingTimeout)
    {
      pendingRequest = 0x00;
    }



    deltaTime = Registers[registerUpdateStatusCycle];
    deltaTime = deltaTime * 1000 * 60;

    if (GatewayLink.PendingReqSerial == 0x00 && (millis() > timeSendStatus + 60000) && millis() > lastFormatedTime + 1500)
    {
#if defined(debugOn)
      PrintSerial("status:");
      PrintSerialln(deltaTime);
#endif
      SendStatus(false);
      timeSendStatus = millis();
    }
  }
}
