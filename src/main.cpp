#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include  <avr/io.h> 





uint8_t oldSREG=SREG;



SoftwareSerial nodemcu(5,6);
SoftwareSerial gserial(2,3);


String cmmd="";
String menu=R"""(
  -> Lighting
  -> Temperature
  -> Water Level
  -> Soil Moisture
  -> Pump on/off
 )""";


 String currentChannel="gsm";

#define SIM800_TX 2
#define SIM800_RX 3

#define device_1 11
#define device_2 12

int index=0;
String number="";
String message="";

char incomingByte;
String incomingData;
bool atCommand=true;

  // StaticJsonBuffer<1000> jsonBuffer;


  // JsonObject&  data=jsonBuffer.createObject();




#define fan 9
#define lighting 8

//photocell variables
#define photocellpin A5
int photocellvalue=0;
String lightStatus;

//water level variables
#define waterValuePin A1
int waterLevel = 0;
#define watervaluePower 7


//soil moisture variables
#define soilSensorPower 10
#define soilSensorPin A2
int soilMoistureValue=0;
String moistureStatus;

// temperature variables
float temp;
float sensorValue;
float voltageOut;

float temperatureC;
float temperatureF;

#define tempPin A0

// motor power


void setwifi(){
  currentChannel="wifi";
}

void sendSMS(String response){
  gserial.println("AT+CMGF=1");
  delay(500);
  gserial.println("AT+CMGS=\"+254745787487\"\r\n"); 
  delay(500);
  gserial.println(response);
  delay(500);
  gserial.write((char)26);
}

// void updateSerial(){
//   delay(500);
//   while(Serial.available()){
//     cmmd+=(char)Serial.read();
//     cmmd.trim(); 
//     if(cmmd.equals("s")){
//       sendSMS("hello");

//     }else{
//       gserial.print(cmmd);
//       gserial.println("");

//     }
//   }
//   while(gserial.available()){
//     Serial.write(gserial.read());
//   }
// }

void  gsmSetUp(){
  gserial.begin(9600);
   while(!gserial.available()){
    gserial.println("AT");
    delay(1000);
    Serial.println("Connecting....");

  }
  Serial.println("Connected");
   
  gserial.println("AT+CMGF=1"); //SET SMS TEXT MODE
  delay(1000);
  gserial.println("AT+CNMI=1,2,0,0,0"); //procedure for receiving sms in the network
  delay(1000);
  gserial.println("AT+CMGL=\"REC UNREAD\""); //read unread messages

  Serial.println("Ready to receive commands");

 
  sendSMS("Connected");
  sendSMS(menu);
  delay(100);
  
}
void setup() {
  
  pinMode(fan,OUTPUT);
  pinMode(watervaluePower,OUTPUT);
  digitalWrite(watervaluePower,LOW);
  pinMode(soilSensorPower,OUTPUT);
  pinMode(device_1,OUTPUT);
  pinMode(device_2,OUTPUT);
  pinMode(lighting,OUTPUT);

  digitalWrite(device_1,LOW);
  digitalWrite(device_2,LOW);



 
  nodemcu.begin(9600);
  
 
   Serial.begin(9600); 



// if(currentChannel=="gsm"){
  gsmSetUp();
// }

 
  
}


float getTemperature(){
  temp=analogRead(tempPin);
  temp = temp * 0.0.004882814;
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("C");
  delay(1000);
  return temp;

  // voltageOut = (temp * 5000) / 1024;
  
  // // calculate temperature for LM35 (LM35DZ)
  // temperatureC = voltageOut / 10;
  // temperatureF = (temperatureC - 32.0)*(5.0/9.0);
  // Serial.print("Temperature: ");
  // Serial.print(temperatureF);
  // return temperatureF;

  // calculate temperature for LM34
  //temperatureF = voltageOut / 10;
  //temperatureC = (temperatureF - 32.0)*(5.0/9.0);
}

int getWaterLevel(){
  
  digitalWrite(watervaluePower,HIGH);
  delay(2000);
  waterLevel =analogRead(waterValuePin);
  digitalWrite(watervaluePower,LOW);
  Serial.println(waterLevel);
  Serial.print("Water tank Level; ");
  
  waterLevel=(waterLevel *100)/300;
  Serial.print(waterLevel);
  Serial.println(" %");
  
  return waterLevel;
}


void readSoilSensor(){
 digitalWrite(soilSensorPower,HIGH);
 delay(10);
  soilMoistureValue=analogRead(soilSensorPin);
   digitalWrite(soilSensorPower,HIGH);
   Serial.print("Soil Moisture Value: ");
   Serial.println(soilMoistureValue);

   if(soilMoistureValue > 1000){
    moistureStatus="Very Dry";
   }else if(soilMoistureValue > 500){
    moistureStatus="Moderately Moist";
   }else{
    moistureStatus="Very Wet";
   }
 
}


void startPump(){
  if(getWaterLevel()>50){
    digitalWrite(device_1,LOW);
  
  }
 
   
}
void irrigation(){
  if(soilMoistureValue>900){
    digitalWrite(device_2,LOW);
  }else{
    digitalWrite(device_2,HIGH);
  }

 
   
}
void photocell(){
  
  photocellvalue=analogRead(photocellpin);
  Serial.print("Light intensity value: ");
  Serial.println(photocellvalue,DEC);
  if (photocellvalue>200)
  {
   lightStatus="Dark";
   digitalWrite(lighting,HIGH);
  }else if(photocellvalue>100){
    lightStatus="Low Light";
    digitalWrite(lighting,HIGH);
  }else{
    lightStatus="Well Lit";
    digitalWrite(lighting,LOW);
  }
  
  delay(1000);
}

void receivedMessage(String inputString){
  //get number of sender

  index=inputString.indexOf('"') + 1;
  inputString=inputString.substring(index);
  index=inputString.indexOf('"');
  number=inputString.substring(0,index);


  //get message of the sender
  index=inputString.indexOf("\n") +1;
  message=inputString.substring(index);
  message.trim();
  Serial.println("Message: " + message);
  message.toUpperCase();
  //get menu

  if(message.indexOf("COMMANDS")> -1){
  
    String response=menu;
    Serial.println(response);
    sendSMS(response);
  }

   if(message.indexOf("TEMPERATURE")> -1){
  
    String response=String(temp);
    // Serial.println(response);
    // response.append("\xB0");
    // sendSMS("Current temperature is: " +  response + "C");
    sendSMS(String(temp));
  }
  if(message.indexOf("WIFI")> -1){
    currentChannel="wifi";

    // String response=String(temp);
    // Serial.println(response);
    // response.append("\xB0");
    sendSMS("Using Wifi");
  }

   if(message.indexOf("WATER LEVEL")> -1){
  
    String response=String(waterLevel);
    Serial.println(response);
    
    sendSMS("Current Water Level is: " +  response + "%");
  }

     if(message.indexOf("LIGHTING")> -1){
  
    String response=String(photocellvalue);
    Serial.println(response);
    sendSMS("Light Intensity: " +  response + ".\nStatus: " + lightStatus);
  }


   if(message.indexOf("SOIL MOISTUR")> -1){
  
    String response=String(soilMoistureValue);
    Serial.println(response);
    sendSMS("Soil Moisture Value: " +  response + ".\nMoisture Status: " + moistureStatus);
  }


  if(message.indexOf("PUMP OFF")> -1){
    digitalWrite(device_1,HIGH);
    // delay(1000);
    String response="Command: Pump Turn off";
    Serial.println(response);
    sendSMS(response);
  }

  
   if(message.indexOf("PUMP ON")> -1){
    digitalWrite(device_1,LOW);
    String response="Command: Pump Turn on";
    Serial.println(response);
    sendSMS(response);
  }

  

  if(message.indexOf("IRRIGATION OFF")> -1){
    digitalWrite(device_2,HIGH);
    // delay(1000);
    String response="Command: Device 2 Turn off";
    Serial.println(response);
    sendSMS(response);
  }


   if(message.indexOf("IRRIGATION ON")> -1){
    digitalWrite(device_2,LOW);
   
   String response="Command: Device 2 Turn on";
    Serial.println(response);
    sendSMS(response);
  }
  // else{
  //   sendSMS("invalid option");
  //   return;
  // }
  delay(50);
}


void WifiCommuication(){

 StaticJsonBuffer<1000> jsonBuffer;


  JsonObject&  data=jsonBuffer.createObject();

  

  data["soilMoisture"] = soilMoistureValue;
  data["photocellvalue"] = photocellvalue;
  data["waterLevel"] = waterLevel;
  data["temperature"] = temp;


  data.printTo(nodemcu);
  jsonBuffer.clear();

  delay(2000);




  
 

}

void GsmCommunication(){
 
   if(gserial.available()){
    delay(100);
   
   
    //serial buffer
    while(gserial.available()){
      incomingByte=gserial.read();
      incomingData+=incomingByte;

      
    }

    delay(10);

    if(atCommand==false){
      receivedMessage(incomingData);

    }else{
      atCommand=false;
    }

    //delete messages to save memory

    if(incomingData.indexOf("OK") == -1){
      gserial.println("AT+CMGDA=\"DEL ALL\"");
      delay(1000);
      atCommand=true;
    }
    incomingData="";
    
    

  }
   
}

void loop() {
  


  // startFan();
 
   getWaterLevel();
  readSoilSensor();
  photocell();
  getTemperature();
   startPump();
   irrigation();
   
  
  Serial.println("currentChannel :" + currentChannel);
  Serial.println("-------------------------------------------------");
  if(currentChannel =="gsm"){
    GsmCommunication();
    // currentChannel="wifi";
    // delay(1000);

  }
 else{
  WifiCommuication();
  // currentChannel="gsm";
  //   delay(3000);
  // StaticJsonBuffer<1000> jsonBuffer;


  // JsonObject&  data=jsonBuffer.createObject();

  

  // data["soilMoisture"] = soilMoistureValue;
  // data["photocellvalue"] = photocellvalue;
  // data["waterLevel"] = waterLevel;
  // data["temperature"] = temp;


  // data.printTo(nodemcu);
  // jsonBuffer.clear();
  // Serial.println("Sent to esp");
  //  GsmCommunication();
  // delay(2000);

 }
// delay(6000);

}



void stopFan(){
  digitalWrite(fan,LOW);
  }
