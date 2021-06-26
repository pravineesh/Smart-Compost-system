//------final code-----
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>

#include <ArduinoJson.h>
#include <DHT.h> //DHT and Adafruit Sensor library(https://github.com/adafruit/Adafruit_Sensor)
#include <Ticker.h> //https://github.com/sstaub/Ticker
#include <ESP32_Servo.h> //adding servo motor library
//------Arduino part-------
//-------DHT---------
int upperShutterStop;
int minimumTemperature = 35; // variable created for minimum temperature
int maximumTemperature = 45; // variable created for maximum temperature
int shutterOpenTemperature =38; //variable created to store temparature value to open upper shutter
//------soil moisture-------
int soilMoisturesensorValue;  // declared a variable to store value of soil moisture sensor
float soilMoisturePercentage; // declared a variable to store converted value of soil moisture sensor to percentage
int moistureMinimum = 40; // minimum moisture value
int moistureMaximum = 60; // maximum moisture value
//---------SERVO----------
int servoPositionBrownWaste = 0;// varriable created to store initial positon of servo(brown waste)
int delayTimeBWServo = 25; // created a varriable to store delay time of servo during operation
int delayTimeBWServoAdd = 2500; //delay for adding  brown waste
int startingAngleBW = 15; // starting angle of rotation of servo motor
int endingAngleBW = 170; // ending angle of rotation of servo motor
Servo brownWaste; // create a servo object called brown waste
//---------
#define DHTPIN 4
#define RELAY1 2//water pump
//---mixer Shaft----//
#define RELAY2 19//mixer shaft
int IN3 = 18; //IN3 of l298n
int IN4 = 5; // IN4 of l298n
//-------//
#define BROWNWASTE 13
//---uppershutter-----//
#define UPPERSHUTTER 15
int IN1 = 27; // IN1 of l298n (GPIO27)
int IN2 = 26; // IN2 of l298n (GPIO26)
 
//-----------//
int SOILMOISTURE = 36; // defining pin 36(ADC0/svn) as soilmoisture
int MQ4 = 39; //defining pin 39
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

void send_sensor();

Ticker timer;

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>

<script>

var connection = new WebSocket('ws://'+location.hostname+':81/');

var button_1_status = 0;
var button_2_status = 0;//mixer
var in3_status = 0;// direction of mixer
var in4_status = 0;// direction of mixer
var button_3_status = 0; // store status of button 3(brown waste)-ON/OFF
var button_4_status = 0;
var in1_status = 0;// direction of upper shaft
var in2_status = 0;// direction of upper shaft
var button_5_status =0; //for turning off upper shutter motor
var temp_data = 0;
var hum_data = 0;
var sMoisture_data = 0; //varriable to store soil moisture data and initialized it to zero
var mq4_data = 0; // created a varriable to store mq4 data and intialized it with value zero
connection.onmessage = function(event){

  var full_data = event.data;
  console.log(full_data);
  var data = JSON.parse(full_data);
  temp_data = data.temp;
  hum_data = data.hum;
  sMoisture_data = data.soil; // sending data from soil to varriable sMoisture_data
  mq4_data = data.gas; // sending data from methane gas sensor to mq4_data
  document.getElementById("temp_meter").value = temp_data;
  document.getElementById("temp_value").innerHTML = temp_data;
  document.getElementById("hum_meter").value = hum_data;
  document.getElementById("hum_value").innerHTML = hum_data;
  /*----soil moisture data and id created------*/
  document.getElementById("sMoisture_meter").value = sMoisture_data;
  document.getElementById("sMoisture_value").innerHTML = sMoisture_data;
/*---MQ4 sensor---*/
  document.getElementById("mq4_meter").value = mq4_data;
  document.getElementById("mq4_value").innerHTML = mq4_data;
}

function button_1_on()
{
   button_1_status = 1; 
  console.log("RELAY 1 is ON");
  send_data();
}

function button_1_off()
{
  button_1_status = 0;
console.log("RELAY 1 is OFF");
send_data();
}

function button_2_on()
{
   button_2_status = 1;
   in3_status = 1;
   in4_status = 0;  
  console.log("RELAY 2 is ON");
  send_data();
}

function button_2_off()
{
  button_2_status = 0;
  in3_status = 0;
console.log("RELAY 2 is OFF");
send_data();
}

function button_3_on()
{
   button_3_status = 1; 
  console.log("RELAY 3 is ON");
  send_data();
}

function button_3_off()
{
  button_3_status = 0;
console.log("RELAY 3 is OFF");
send_data();
}
//----closing uppershutter
function button_4_on()
{
   button_4_status = 1;
   in1_status = 1;
   in2_status = 0; 
  console.log("upper shutter closing");
  send_data();
}

function button_4_off()
{
  button_4_status = 1;
  in1_status = 0;
  in2_status = 1;
console.log("uppershutter opening");
send_data();
}
function button_5_off()
{
  button_4_status = 0;
  in1_status = 0;
  in2_status = 0;
console.log("RELAY 4 is OFF(upper shutter)");
send_data();
}
function send_data()
{
  var full_data = '{"RELAY1" :'+button_1_status+',"RELAY2":'+button_2_status+',"BROWNWASTE":'+button_3_status+',"UPPERSHUTTER":'+button_4_status+',"IN1":'+in1_status+',"IN2":'+in2_status+',"UPPERSHUTTER":'+button_5_status+',"IN3":'+in3_status+'}';
  connection.send(full_data);
}


</script>
<body style="background-color:#F5FFFA;">

<center>
<h1>Smart <span style="color:darkolivegreen;font-weight:bold">Compost</span></h1>
<h4>A fully automated compost system</h4>
<div style="background-color: lightgrey; width: 300px; border: 15px ; border-radius: 12px; padding: 50px; margin: 20px;">
<h3> WATER PUMP </h3>
<button onclick= "button_1_on()" >On</button><button onclick="button_1_off()" >Off</button>
<h3> MIXER SHAFT </h3>
<button onclick="button_2_on()">On</button><button onclick="button_2_off()">Off</button>
<!--<h3> BROWN WASTE </h3>
button onclick="button_3_on()">On</button><button onclick="button_3_off()">Off</button>-->
<h3> UPPER SHUTTER </h3>
<button onclick="button_4_on()">Close</button><button onclick="button_4_off()">Open</button><button onclick="button_5_off()">Stop</button>

</div>
<div style="background-color: lightgrey; width: 300px; border: 15px ; border-radius: 12px; padding: 50px; margin: 20px;">
<div style="text-align: center;">
<h3>Temperature</h3><meter value="2" min="0" max="100" id="temp_meter"> </meter><h3 id="temp_value" style="display: inline-block;"> 2 </h3>
<h3>Humidity</h3><meter value="2" min="0" max="100" id="hum_meter"> </meter><h3 id="hum_value" style="display: inline-block;"> 2 </h3>

<h3>Soil Moisture</h3><meter value="2" min="0" max="100" id="sMoisture_meter"> </meter><h3 id="sMoisture_value" style="display: inline-block;"> 2 </h3>
<h3>Methane Gas</h3><meter value="2" min="0" max="100" id="mq4_meter"> </meter><h3 id="mq4_value" style="display: inline-block;"> 2 </h3>
</div>
</center>
</body>
</html>

)=====";

// ipaddress/led1/on
//ipaddress/led1/off

// ipaddress/led2/on
//ipaddress/led2/off
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int RELAY1_status = doc["RELAY1"];
  int RELAY2_status = doc["RELAY2"];
  int BROWNWASTE_status = doc["BROWNWASTE"];
  int UPPERSHUTTER_status = doc["UPPERSHUTTER"];
  int IN1_status = doc["IN1"];
  int IN2_status = doc["IN2"];
  int IN3_status = doc["IN3"];
  
 
  digitalWrite(RELAY1,RELAY1_status);
  digitalWrite(RELAY2,RELAY2_status);
  digitalWrite(BROWNWASTE,BROWNWASTE_status);
  digitalWrite(UPPERSHUTTER,UPPERSHUTTER_status);
  digitalWrite(IN1,IN1_status);
  digitalWrite(IN2,IN2_status);
  digitalWrite(IN3,IN3_status);


  }
}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);
  pinMode(BROWNWASTE,OUTPUT);
  pinMode(UPPERSHUTTER,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pinMode(SOILMOISTURE,INPUT);
  pinMode(MQ4,INPUT);
  dht.begin();
  brownWaste.attach(BROWNWASTE);
  servoPositionBrownWaste = startingAngleBW;
  upperShutterStop = 1;
 
  WiFi.softAP("smartCompost", "");
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("smartcompost")) { //esp.local/
    Serial.println("MDNS responder started");
  }



  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

   server.on("/RELAY1/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(RELAY1,HIGH);
  request->send_P(200, "text/html", webpage);
  });

  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);
  timer.attach(2,send_sensor);
 // timer.attach(10,soilMoisture);
 //timer.attach(10,temperatureControl);

}


void loop(void)
{
 websockets.loop();
 //soilMoisture();
 //temperatureControl();
}

void send_sensor()
{
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  //read soil moisture data
  float soilM =analogRead(SOILMOISTURE);
 float s =( 100 - ( (soilM/4095.00) * 100 ) );
  //read MQ4 data
  float g = analogRead(MQ4);
  float m =( 100 - ( (g/4095.00) * 100 ) );
    if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // JSON_Data = {"temp":t,"hum":h,"soil":s,"gas":g}
  String JSON_Data = "{\"temp\":";
         JSON_Data += t;
         JSON_Data += ",\"hum\":";
         JSON_Data += h;
         JSON_Data += ",\"soil\":";
         JSON_Data += s;
         JSON_Data += ",\"gas\":";
         JSON_Data += m;
         JSON_Data += "}";
   Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);

  soilMoisture();
  
}
//soil moisture
void soilMoisture()
{
  soilMoisturesensorValue = analogRead(SOILMOISTURE);  //read soil moisture sensor value
    soilMoisturePercentage = ( 100 - ( (soilMoisturesensorValue/4095.00) * 100 ) ); // convert it to percentage
    Serial.print("soil moisture: ");
    //for starting error
    if(soilMoisturePercentage==100)
    {
      soilMoisturePercentage=soilMoisturePercentage-50;
      }
    Serial.print( soilMoisturePercentage); // print humidity percentage on screen
    Serial.println("% ");
    //turning on dc motor pump if soil moisture is below 40%

    if(soilMoisturePercentage<=moistureMinimum)
    {
      digitalWrite(RELAY1,HIGH);
      Serial.println("soil is dry");
      Serial.println("water pump is ON");
      delay(1000);
      
      }
      else
     {
      digitalWrite(RELAY1,LOW);
      }
     
     // adding brown waste and operatint mixer shaft if humidity is high
     
     if(soilMoisturePercentage>=moistureMaximum)
     {
      digitalWrite(RELAY1,LOW);//turning off pump
      Serial.println("soil moisture is high");
      Serial.println("adding brown waste and operating mixershaft");
      delay(delayTimeBWServo);
     
      for(servoPositionBrownWaste;servoPositionBrownWaste<=100;servoPositionBrownWaste=servoPositionBrownWaste+1)
      {
        brownWaste.write(servoPositionBrownWaste);
        delay(delayTimeBWServo);
      
        }
        for(servoPositionBrownWaste;servoPositionBrownWaste>=15;servoPositionBrownWaste=servoPositionBrownWaste-1)
      {
        brownWaste.write(servoPositionBrownWaste);
        delay(delayTimeBWServo);
      
        }
          //dc motor-mixershft operation


        
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
        delay(100);
        digitalWrite(RELAY2,1);
        delay(5000);
        digitalWrite(RELAY2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
     }
      temperatureControl();
}

void temperatureControl()
{

int temperature = dht.readTemperature();
   //turning on the pump if temperature is high
      
    if(temperature>maximumTemperature)
    {
      digitalWrite(RELAY1,HIGH); //turning on water pump pin
      Serial.println("Temperature is high");
      Serial.println("water pump is ON for cooling compost");
       
      }
     else
     {
      digitalWrite(RELAY1,LOW);
      }
    
     // closing upper shutter
     
     
      if(temperature<minimumTemperature) //enter to if condition if temperature is below minimumTemperature
      {
        Serial.println("Temperature is very low");
        Serial.println("upper shutter is closed");
      
       if(upperShutterStop==1)
       {
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        
        digitalWrite(UPPERSHUTTER,HIGH);
        delay(4000);
        digitalWrite(UPPERSHUTTER,LOW);
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        upperShutterStop = 0;
       }
        }
    //opening upper shutter

      if(temperature>shutterOpenTemperature) //enter to if condition if temperature is below minimumTemperature
      {
        Serial.println("upper shutter is open");
        
        
      if(upperShutterStop==0)
      {
       digitalWrite(IN1,LOW);
        digitalWrite(IN2,HIGH);
        
        digitalWrite(UPPERSHUTTER,HIGH);
        delay(3500);
        digitalWrite(UPPERSHUTTER,LOW);
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        upperShutterStop = 1;

      } 
      }
        
}
