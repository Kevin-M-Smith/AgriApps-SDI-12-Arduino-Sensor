#include <SDI12.h>

#define _BUFFER_SIZE 64    
#define DISABLED 0
#define ENABLED 1
#define HOLDING 2
#define TRANSMITTING 3
#define LISTENING 4
#define SPACING 830 

// ------ SETTINGS ------

#define DATAPIN 4 

String sdiVersion = "13";
String companyName = "AGRIAPPS";
String sensorModelNumber = "100000";
String sensorVersion = "1.2";
String serialNumber = "101";

// ------ END SETTINGS ------

SDI12 mySDI12(DATAPIN); 

String commandReceived = "";

char sensor1 = 'a';
char sensor2 = 'b';
char sensor3 = 'c';
char sensor4 = 'd';
char sensor5 = 'e';
char sensor6 = 'f';
char sensor7 = 'g';
char sensor8 = 'h';

char currentAddress = 'a';
int currentReadingDelay = 2;    // Default to 2 seconds


void setup(){
  
  Serial.begin(9600);
  Serial.println("SDI-12 sensor");

  delay(2000);

  mySDI12.begin(); 
  
  delay(500); // allow things to settle
  
  setState(LISTENING);


}

void loop(){

  while(mySDI12.available()){

    char charReceived = (char)mySDI12.read();  
    Serial.println(charReceived);

    if (charReceived == '!'){  
    
      Serial.println(commandReceived);
      
      parseCommand(commandReceived);
      
      commandReceived = "";
      
    }else{
      commandReceived += charReceived;
    }

  }

}


// ------ State machine to signal to Master when transmitting etc -----

void setState(uint8_t state){
  
  if(state == HOLDING){
    pinMode(DATAPIN,OUTPUT);
    digitalWrite(DATAPIN,LOW);
    *digitalPinToPCMSK(DATAPIN) &= ~(1<<digitalPinToPCMSKbit(DATAPIN));
    return; 
  }
  
  if(state == TRANSMITTING){
    pinMode(DATAPIN,OUTPUT);
    noInterrupts();       
    return; 
  }
  
  if(state == LISTENING) {
    digitalWrite(DATAPIN,LOW);
    pinMode(DATAPIN,INPUT); 
    interrupts();       
    *digitalPinToPCICR(DATAPIN) |= (1<<digitalPinToPCICRbit(DATAPIN));
    *digitalPinToPCMSK(DATAPIN) |= (1<<digitalPinToPCMSKbit(DATAPIN));
    
  }else{          
    digitalWrite(DATAPIN,LOW); 
    pinMode(DATAPIN,INPUT);
    *digitalPinToPCMSK(DATAPIN) &= ~(1<<digitalPinToPCMSKbit(DATAPIN));
    if(!*digitalPinToPCMSK(DATAPIN)){
      *digitalPinToPCICR(DATAPIN) &= ~(1<<digitalPinToPCICRbit(DATAPIN));
    }
  }
}

void sendResponse(String response){
      
  sendResponseUsingAddress(currentAddress, response + "\r\n");

}

void sendResponseUsingAddress(char address, String response){

  Serial.println("Sending response: " + currentAddress + response);
  
  setState(TRANSMITTING);
  mySDI12.sendCommand(currentAddress + response + "\r\n");

  // MAKE LED FLASH HERE?
 
  setState(LISTENING);

}

void parseCommand(String command){

  // First char of command is always the address of the device being probed

  // ---- FROM SDI-12 SPEC ----
  // ASCII '0' through ASCII '9' are the standard addresses which all sensors and data
  // recorders must support. Should there be a need for more than 10 sensors, use an address in the
  // range ASCII 'A' through ASCII 'Z' (decimal 65 through 90) and ASCII 'a' through ASCII 'z'
  // (decimal 97 through 122). 
  //
  // It can also be a ? if Master is querying the device's address
  //
  // So possible addresses are 0-9, a-z, A-Z, and ?

  currentAddress = command.charAt(0);

  Serial.println("currentAddress: " + currentAddress);

  if(command.length() == 1){

    // ----- This is Acknowledge Active request -----
    acknowdgeActive();
    
  }else{

    String action = command.substring(1);
  
    if(action.charAt(0) == 'I'){
      // ----- This is a Send Identification action -----
      sendIdentification();
      
    }else if(action.charAt(0) == 'A'){
      // ----- This is a Change Address action -----
      
      changeAddress(action.charAt(1));
      
    }else if(action.charAt(0) == '?'){
      // ----- This is and Address Query action -----
      addressQuery();
      
    }else if(action.charAt(0) == 'M'){
      // ----- This is Start Measurement action -----
      startMeasurement();
      
    }else if(action.charAt(0) == 'D'){
      // ----- This is Send Data action -----
      sendData(int(action.charAt(1)));
      
    }else if(action.charAt(0) == 'V'){
      // ----- This is Start Verification action -----
      // TODO
      
    }else if(action.charAt(0) == 'C'){
      // ----- This is Start Concurrent Measurement action -----
      // TODO
      
    }else if(action.charAt(0) == 'R'){
      // ----- This is a Continuous Measurements action -----
      // TODO
      
    }else{
      // Unknown request
      Serial.println("ERROR: Unknown command");
    }
  }
  
}
    

void acknowdgeActive(){

    // ------ Acknowledge Active ------
    Serial.println("acknowledgeActive");
    sendResponse("");
      

}

void sendIdentification(){
  
  // ------ Send Identification ------
      
  String response = "" + sdiVersion + companyName + sensorModelNumber + sensorVersion + serialNumber;
  Serial.println("sendIdentification: " + response);
  
  sendResponse(response);
  
}


void changeAddress(char newAddress){
  
    // ------ Change Address ------
    // TODO - Actually swap the addresses

    sendResponseUsingAddress(newAddress, "");

}

void addressQuery(){
  // ------- Address Query -------
  Serial.println("addressQuery");
  sendResponse("");
}


void startMeasurement(){
    // ------ Start Measurement -------
    // ttt - number of seconds till measurement is ready
    // d - number the number of measurement values the sensor will make and return
    // in one or more subsequent D commands; n is a single digit integer with
    // a valid range of 1 to 9

    // TODO - Defaulting to 2 seconds

    String response = "";
  
    currentReadingDelay = 2;
    int d = 0;

    if(currentReadingDelay < 10){
      response = "00" + currentReadingDelay;
    }else if(currentReadingDelay >= 10 && currentReadingDelay < 100){
      response = "0" + currentReadingDelay;
    }else if (currentReadingDelay >= 100){
      response = "" + currentReadingDelay;
    }else{
      Serial.println("ERROR: Invalid ttt for sensor " + currentAddress);
    }

    response = response + d;
    Serial.println("startMeasurement: " + response);
    sendResponse(response);

    

}

void startMeasurementWithCRC(){
    // ------ Start Measurement and Request CRC ------
    // TO BE IMPLEMENTED
    
}
 
    

void sendData(int measurementNumber){

  // ------- Send Data ------

  String data = "{'data': {'sensorType': 'atmospheric-temperature', 'value': 25.00}}";
  Serial.println("sendData: " + data);
  sendResponse(data);

}
 
void additionalMeasurements(){

  // ------ Additional Measurements ------
  // TODO
 
}

