#include <XBee.h>

XBee xbee = XBee();
uint8_t chCmd[] = {'C','H'};
uint8_t chValue[16][1] = { {0x0B}, {0x0C}, {0x0D}, {0x0E}, {0x0F}, {0x10}, {0x11}, {0x12}, {0x13}, {0x14}, {0x15}, {0x16}, {0x17}, {0x18}, {0x19}, {0x1A} };
int taille = 100, rssiArray[5][100];

AtCommandRequest chGetValue = AtCommandRequest(chCmd);
AtCommandResponse atResponse = AtCommandResponse();
Rx16Response rx16 = Rx16Response();

void sendCommand(AtCommandRequest atRequest, String message = "Command value: ", bool debug = false);

int rssi, result, allCheck, pwrLvl, oldPwrLvl, counter;

void setup(){
  Serial.begin(9600);
  result = 0;
  allCheck = 0;
  counter = 0;
  oldPwrLvl = 0;
  AtCommandRequest chSetValue = AtCommandRequest(chCmd, chValue[15], sizeof(chValue[15]));
  sendCommand(chSetValue);
  sendCommand(chGetValue, "Channel number: ");
  Serial.println(allCheck);
}

void loop()
{
  xbee.readPacket(100);
  if (xbee.getResponse().isAvailable() && allCheck < 6){
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE){
      xbee.getResponse().getRx16Response(rx16);
      rssi = rx16.getRssi();
      pwrLvl = rx16.getData(0);
      //Serial.println(pwrLvl);
      if(pwrLvl != oldPwrLvl){
        oldPwrLvl = pwrLvl;
        counter = 0;
        allCheck++;
        Serial.println("allCheck = " + String(allCheck) + " Power level = " + String(pwrLvl));
      }else counter ++;
      if(allCheck < 6){
        if(counter <= 100){
          rssiArray[pwrLvl][counter] = rssi;
          //Serial.println(counter);
        }
        if(counter == 100){
          Serial.println("--> All messages received!");
        }
      }
    }
  }else if(allCheck == 6){
    Serial.println();
    Serial.println("### Resultats ###");
    Serial.println();
    for(int  i = 0; i < 5; i++){
      for(int t = 0; t < taille; t++){
        result = result + rssiArray[i][t];
      }
      result = result / taille;
      Serial.println("Moyenne pour pwrLvl " + String(i) + " est de " + String(result));
    }
    while(1){};
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
