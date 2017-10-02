#include <XBee.h>
#include <EEPROM.h>

XBee xbee = XBee();
uint8_t plCmd[] = {'P','L'};
uint8_t chCmd[] = {'C','H'};
//uint8_t message[] = { 'S', 'a', 'l', 'u', 't' };
uint8_t plValue[5][1] = { {0x0}, {0x1}, {0x2}, {0x3}, {0x4} };
uint8_t message[5][1] = { {0x0}, {0x1}, {0x2}, {0x3}, {0x4} };
uint8_t chValue[16][1] = { {0x0B}, {0x0C}, {0x0D}, {0x0E}, {0x0F}, {0x10}, {0x11}, {0x12}, {0x13}, {0x14}, {0x15}, {0x16}, {0x17}, {0x18}, {0x19}, {0x1A} };

XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x00A2);
AtCommandRequest plGetValue = AtCommandRequest(plCmd);
AtCommandRequest chGetValue = AtCommandRequest(chCmd);
AtCommandResponse atResponse = AtCommandResponse();
Tx16Request tx16;

void sendCommand(AtCommandRequest atRequest, String message = "Command value: ", bool debug = false);
void messageForRSSI(int maxMessage = 100);

int count, address = 0;
byte eeprom;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  AtCommandRequest chSetValue = AtCommandRequest(chCmd, chValue[15], sizeof(chValue[15]));
  sendCommand(chSetValue);
  sendCommand(chGetValue, "Channel number: ");

  eeprom = EEPROM.read(address);
  if(eeprom == 0){
    EEPROM.write(address, 1);
  }else if(eeprom == 1){
    EEPROM.write(address, 2);
  }else if(eeprom == 2){
    EEPROM.write(address, 3);
  }else if(eeprom == 3){
    EEPROM.write(address, 4);
  }else if(eeprom == 4){
    EEPROM.write(address, 0);
  }else EEPROM.write(address, 0);
  Serial.println(eeprom);

  tx16 = Tx16Request(addr64, message[eeprom], sizeof(message[eeprom]));
  AtCommandRequest plSetValue = AtCommandRequest(plCmd, plValue[eeprom], sizeof(plValue[eeprom]));
  sendCommand(plSetValue);
  sendCommand(plGetValue, "Power level: ");

}

void loop() {
  messageForRSSI(100);
  while(1){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
}

void sendCommand(AtCommandRequest atRequest, String message = "Command value: ", bool debug = false){
  xbee.send(atRequest);
  if (xbee.readPacket(5000)) {
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        if(debug){
          Serial.println();
          Serial.print("Command [");
          Serial.print(atResponse.getCommand()[0]);
          Serial.print(atResponse.getCommand()[1]);
          Serial.println("] was successful!"); 
        }
        if (atResponse.getValueLength() > 0) {
          if(debug){
            Serial.print("Command value length is ");
            Serial.println(atResponse.getValueLength(), DEC);
          }
          Serial.print(" " + message);
          for (int i = 0; i < atResponse.getValueLength(); i++) {
              Serial.println(atResponse.getValue()[i]);
          }
        }
      } 
      else {
        if(debug){
          Serial.print("Command return error code: ");
          Serial.println(atResponse.getStatus(), HEX);
        }
      }
    } else {
      if(debug){
        Serial.print("Expected AT response but got ");
        Serial.print(xbee.getResponse().getApiId());      
      }
    }   
  } else {
    if (xbee.getResponse().isError()) {
      if(debug){
        Serial.print("Error reading packet.  Error code: ");  
        Serial.println(xbee.getResponse().getErrorCode());
      }
    } 
    else {
      if(debug){
        Serial.print("No response from radio");
      }  
    }
  }
}

void messageForRSSI(int maxMessage){
  if(maxMessage > 900) maxMessage = 900;
  count = 0;
  while(count < maxMessage + 10){
    xbee.send(tx16);
    Serial.println();
    delay(750);
    count++;
  }
  Serial.println("### FIN DE L'EMISSION " + String(count) + " messages ###");
}

