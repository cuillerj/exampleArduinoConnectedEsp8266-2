/*
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Written by J Cuiller
*/
/*
   This is a software that can be used as an Arduino example It is based on a framework.
   It allows UDP communication with a Java server thru ESP8266 WIFI.
   It provides the following standard functions:
   -   key values (indicators) and registers are stored in Eeprom
   -   the services address (IP address,UDP port) are dynamicaly updated
   -   set the microcontroler date and time according to the server
   -   regurarly send status to keep the server informed
   -   regurarly send indicators values that are stored in database
   -   regurarly send mesurments values that are stored in database
   -   regurarly send mesurments values that are stored in google sheet
   -   regurarly send registers values that are transfered to a dedicated software
   -   indicators values are updated on request of the server
*/
/*
   version v02 - config PIN inverted
   version v03 - serial link modified to allow esp8266 shield on ATMega
*/
/*
   comment below all define debug when using Arduino with only one serial link (Uno, Nano..)
   when using multiple serial link (AtMega...) it is possible to use one serial link to print for debugging
   when using a home made Esp8266 gateway serialx (defined in SerialLink.h) is used to exchange with the gateway and debug use serial0 link /usb
   when using a Esp8266 shield gateway serial0 is used to exchange with the gateway and debug use (defined in SerialLink.h) needing to add a ftdi to use usb
*/
//#define debugOn  // debug can on only be used with multiple serial interfaces Arduino
//#define debugConnection // low level debug
//#define debugEeprom

#define forceInitEeprom false


/*
   Date Time managment
   uncomment either TimeMode or RTCMode to use up to date and time
   uncomment RTCMode in case you a RTC DS1307 hardware is connected to the arduino
   uncomment TimeMode in case of no specific hardware clock
*/

#define TimeMode
//#define RTCMode

#ifdef RTCMode
#include <RTClib.h>
RTC_DS1307 RTC;
#define TimeOn
#endif

#ifdef TimeMode
#include <TimeLib.h>
#define TimeOn
#endif

#define Version "framework"
#define ver 0x03 // version 
#include <EEPROM.h>  // to read and write parameters from/to eeprom
#include <SerialLink.h> // software that manage the serial link communication level
#include <HomeAutomationBytesCommands.h> // commands specifications
#define PendingReqRefSerial  routeFrame; // request to the gateway means route (ready to eventualy add some more action in the gateway)
//#define receiveFlagPosition 0     //
#define gatewayLinkSpeed 38400   // 
/*

*/
boolean readyToRun = true;
uint8_t unitGroup = 0x05; // default value that will be overwritten by the value read from eeprom unless Arduino is in config mode. If so this value will be stored in Eeprom
uint8_t unitId = 0x0f; // default value that will be overwritten by the value read from eeprom unless Arduino is in config mode. If so this value will be stored in Eeprom
#define maxValuesToSend 7  // limitation due to the serial link
boolean gatewayPowerOnStatus = false;
uint8_t trameNumber = 0;     // frame number to send
#define numberFrameSource 2 // at least 1 for the dispatcher (frame source can be 0x00 or 0x01
uint8_t lastAckTrameNumber[numberFrameSource];  // last frame number acknowledged by the server
uint8_t pendingAckSerial = 0;    // flag waiting for acknowledged
int retryCount = 0;            // number of retry for sending
uint8_t pendingRequest = 0x00;
uint8_t gatewayStatus = 0x00;    // 0x00 not ready  0x01 ready GPIO on wait boot completed 0x02 ready

#define pendingTimeout 60000           // not wait for answer to a request longer that this timer
#define updateClockCycle 600000        // time request using this cycle
#define updateClockLimit 1300000        // limit duration without receiving time information over that time bit error is raised 
#define updateSheetCycle 200        // time request using this cycle /sec
#define updateRegisterCycle 120000    // millis sec
#define updateIndicatorCycle 100000    // millis sec
#define updateStatusCycle 5       // time request using this cycle / mn
/*
  diagByte description
*/
#define diag0 0
#define diagTimeUpToDate 2
#define diagServerConnexion 3
#define diagGatewayReady 4
#define diagEeprom 7
#define GatewayReadyPIN 7 // can be used to check that ES82666 is ready and connected to WIFI
#if defined GatewayReadyPIN
#define diagInitMask 0b10010100
#else
#define diagInitMask 0b10000100
#endif
uint8_t diagByte = 0x00 | diagInitMask;
unsigned long lastReceivedTime;
unsigned long lastFormatedTime;
unsigned long timeSendSecSerial;  // used to check for acknowledgment of secured frames
unsigned long timeSendStatus;  // used to regularly send status
unsigned long timeReceiveSerial;  // used to regularly check for received message
unsigned long lastUpdateClock = updateClockCycle + 5000;
unsigned long pendingRequestClock;
unsigned long timeInsertGoogleSheet;
unsigned long timeInsertDatabase = 5000;
unsigned long timeSendRegister;
unsigned long timeSendIndicator;
unsigned long deltaTime;

//#if defined TimeOn
#define MonthList "JanFebMarAprMayJunJulAugSepOctNovDec"  // do not change can not be localized
//#endif
#define eepromVersion 0x01
#define eepromVersionAddr 0
#define eepromUnitIdAddr eepromVersionAddr + 1
#define eepromRegistersAddr 3
/*
  GPIO
*/

#define configPIN MOSI     // when high configuration will be stored in Eeprom during setup - MISO PIN can easely be set with a switch on ISCP connector
#define configLED 13
#define gatewayReadyLED 13
#define relayPIN 6
/*

*/
#define registerUpdateSheetCycle 0
#define registerUpdateStatusCycle 1
#define registerSize 2
#define indicatorsSize 3
uint8_t Registers[registerSize] = {0x00, updateStatusCycle};
int Indicators[indicatorsSize] = {updateSheetCycle, updateStatusCycle};
uint8_t indicator6 = 1;
SerialLink GatewayLink(gatewayLinkSpeed);   // define the object link to the gateway
void setup() {
  // put your setup code here, to run once:
  pinMode(configPIN, INPUT_PULLUP);
  pinMode(configLED, OUTPUT);
  pinMode(relayPIN, OUTPUT);
  digitalWrite(configLED, 0);
#if (defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)) && not defined(esp8266Shield)
  Serial.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(100);
  Serial.println("start usb print");
#endif
#if (defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)) && defined(esp8266Shield)
#if defined(serial1)
  Serial1.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(100);
  Serial1.println("start usb print");
#endif
#if defined(serial2)
  Serial2.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(100);
  Serial2.println("start usb print");
#endif
#if defined(serial3)
  Serial3.begin(38400);            // use serial for debug only with Arduino that has multiple serial interfaces
  delay(100);
  Serial3.println("start usb print");
#endif
#endif

#if defined(debugOn)
  delay(100);
  PrintSerial(Version);
  PrintSerial(":");
  PrintSerialln(ver);
  PrintSerial("config Mode:");
  PrintSerialln(digitalRead(configPIN));
#endif
  if (!digitalRead(configPIN) || forceInitEeprom)
  {
    digitalWrite(configLED, 1);
    InitConfiguration();
    digitalWrite(configLED, 0);
    readyToRun = false;
  }
  LoadParameters();
#if defined GatewayReadyPIN
  pinMode(GatewayReadyPIN, INPUT);
#endif
  GatewayLink.SerialBegin();
#if defined(debugConnection)
  Serial.print("serial link:");
  Serial.println(GatewayLink.SerialNumber());
#endif
#if defined(debugOn)
  PrintSerial("group:");
  PrintSerial(unitGroup);
  PrintSerial(" unit:");
  PrintSerialln(unitId);
#endif
#if defined RTCMode
  {
    RTC.begin();
    DateTime now = RTC.now();
  }
#endif
  randomSeed(analogRead(7));
  SendStatus(true);
}

void loop() {
  delay(100);
  while (readyToRun) {
    StandardLoop();
    if (GatewayLink.PendingReqSerial == 0x00 && (millis() > timeSendRegister + updateRegisterCycle + 2000) && millis() > lastFormatedTime + 1500)
    {
#if defined(debugOn)
      PrintSerialln("send register");
#endif
      SendRegisters();
      timeSendRegister = millis();
    }
    if (GatewayLink.PendingReqSerial == 0x00 && (millis() > timeSendIndicator + updateIndicatorCycle + 2000) && millis() > lastFormatedTime + 1500)
    {
#if defined(debugOn)
      PrintSerial("send indicators:");
#endif
      SendIndicators();
      timeSendIndicator = millis();
    }
    if (GatewayLink.PendingReqSerial == 0x00 && (millis() > timeInsertGoogleSheet + deltaTime) && millis() > lastFormatedTime + 1500)
    {
#define nbValues 7
      int values[nbValues];
      for (int i = 0; i < nbValues; i++)
      {
        values[i] = analogRead(i);
      }
#if defined(debugOn)
      PrintSerial(" sheet:");
      PrintSerialln(deltaTime);
#endif
      SendToGoogleSheet(nbValues, values, true);
      timeInsertGoogleSheet = millis();
    }
    if (GatewayLink.PendingReqSerial == 0x00 && (millis() > timeInsertDatabase + deltaTime) && millis() > lastFormatedTime + 1500)
    {
#define nbValues 7
      int values[nbValues];
      uint8_t type = 0x00;
      for (int i = 0; i < nbValues; i++)
      {
        values[i] = analogRead(i);
      }
#if defined(debugOn)
      PrintSerialln(" data base");
#endif
      SendToDatabase(nbValues, type, values, true);
      timeInsertDatabase = timeInsertGoogleSheet + 5000;
    }
    digitalWrite(relayPIN, Registers[0]);
  }
  /*
     inser specific code below
  */

}
