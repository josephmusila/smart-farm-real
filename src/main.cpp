#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include  <avr/io.h> 





uint8_t oldSREG=SREG;



SoftwareSerial nodemcu(5,6);
SoftwareSerial gserial(2,3);


String cmmd="";
 String menu="Select the Data to Query.\n-----------------------\n-> Lighting\n-> Temperature\n-> Water Level\n-> Soil Moisture\n ";
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

#define tempPin A4

// motor power




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

void setup() {
  
  pinMode(fan,OUTPUT);
  pinMode(watervaluePower,OUTPUT);
  digitalWrite(watervaluePower,LOW);
  pinMode(soilSensorPower,OUTPUT);
  pinMode(device_1,OUTPUT);
  pinMode(device_2,OUTPUT);

  digitalWrite(device_1,LOW);
  digitalWrite(device_2,LOW);



 
  nodemcu.begin(9600);
  gserial.begin(9600);
 
   Serial.begin(9600); 
//   Serial.print("Initializing....");


//  // initialize the sim800l

//   gserial.println("AT");
//   updateSerial();

//   gserial.println("AT+CSQ");
//   updateSerial();

//   gserial.println("AT+CCID");
//   updateSerial();

//   gserial.println('AT+CPIN');
//   updateSerial();

//   gserial.println("AT+COPS?");
//   updateSerial();

//   gserial.println("AT+CBC");
//   updateSerial();

//   gserial.println("AT+CREG?");
//   updateSerial();

//   gserial.println("AT+CMGF=1");
//   updateSerial();

//   gserial.println("AT+CNMI=1,2,0,0,0");
//   updateSerial(); 


//   delay(1000);

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


float getTemperature(){
  temp=analogRead(tempPin);
  temp = temp * 0.48828125;
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("C");
  delay(1000);
  return temp;
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


void startFan(){
  digitalWrite(fan,HIGH);
   
}
void photocell(){
  
  photocellvalue=analogRead(photocellpin);
  Serial.print("Light intensity value: ");
  Serial.println(photocellvalue,DEC);
  if (photocellvalue>200)
  {
   lightStatus="Dark";
  }else if(photocellvalue>100){
    lightStatus="Low Light";
  }else{
    lightStatus="Well Lit";
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

  if(message.indexOf("MENU")> -1){
  
    String response=menu;
    Serial.println(response);
    sendSMS(response);
  }

   if(message.indexOf("TEMPERATURE")> -1){
  
    String response=String(temp);
    Serial.println(response);
    // response.append("\xB0");
    sendSMS("Current temperature is: " +  response + "C");
  }

   if(message.indexOf("WATER LEVEL")> -1){
  
    String response=String(waterLevel);
    Serial.println(response);
    
    sendSMS("Current Water Level is: " +  response + "%");
  }

     if(message.indexOf("LIGHTING")> -1){
  
    String response=String(photocellvalue);
    Serial.println(response);
    sendSMS("Light Intensity Value: " +  response + ".\n Lighting Status: " + lightStatus);
  }


   if(message.indexOf("SOIL MOISTUR")> -1){
  
    String response=String(soilMoistureValue);
    Serial.println(response);
    sendSMS("Current Soil Moisture Value is: " +  response + ".\n Soil Moisture Status: " + moistureStatus);
  }




  //turn device 1 on

  if(message.indexOf("D1 ON")> -1){
    digitalWrite(device_1,HIGH);
    // delay(1000);
    String response="Command: Device 1 Turn on";
    Serial.println(response);
    sendSMS(response);
  }

  // turn device 1 off
   if(message.indexOf("D1 OFF")> -1){
    digitalWrite(device_1,LOW);
    String response="Command: Device 1 Turn off";
    Serial.println(response);
    sendSMS(response);
  }

   //turn device 2 on

  if(message.indexOf("D2 ON")> -1){
    digitalWrite(device_2,HIGH);
    // delay(1000);
    String response="Command: Device 2 Turn on";
    Serial.println(response);
    sendSMS(response);
  }

  // turn device 2 off
   if(message.indexOf("D2 OFF")> -1){
    digitalWrite(device_2,LOW);
   
   String response="Command: Device 2 Turn off";
    Serial.println(response);
    sendSMS(response);
  }
  // else{
  //   sendSMS("invalid option");
  //   return;
  // }
  delay(50);
}


void getData(){

//  StaticJsonBuffer<1000> jsonBuffer;


//   JsonObject&  data=jsonBuffer.createObject();

  

//   data["soilMoisture"] = soilMoistureValue;
//   data["photocellvalue"] = photocellvalue;
//   data["waterLevel"] = waterLevel;
//   data["temperature"] = temp;


//   data.printTo(nodemcu);
//   jsonBuffer.clear();

//   delay(2000);




  startFan();
   getWaterLevel();
  readSoilSensor();
  photocell();
  getTemperature();
   Serial.println(temp);
   

  Serial.println("-------------------------------------------------");
 

}

void loop() {
  // while(true){
    
  //  nodemcu.begin(9600);
//   Serial.println("opened wifi comm");



  

  

  // data["soilMoisture"] = soilMoistureValue;
  // data["photocellvalue"] = photocellvalue;
  // data["waterLevel"] = waterLevel;
  // data["temperature"] = temp;


//   data.printTo(nodemcu);
//   jsonBuffer.clear();

//   delay(2000);




  // startFan();
   getWaterLevel();
  readSoilSensor();
  photocell();
  getTemperature();
   Serial.println(temp);

  // Serial.println("-------------------------------------------------");
  
   

  

//  Serial.println(currentDateTime());  
 
  
 
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



void stopFan(){
  digitalWrite(fan,LOW);
  }

  // Arduino’s are microcontrollers without operating systems or any kind of persistent storage,
  //  so lack anything that could remember the data and time.
  //  The best you can do is the mills() function that give the number of milliseconds since the Arduino was last reset.