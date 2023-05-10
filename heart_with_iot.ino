#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//SoftwareSerial ser(9,10);
String apiKey = "UOCTLBP3QBTK5UAM";


#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   
#define tempPin A1

//  Variables
const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 
int led=2;
int buzzer=11;                               
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

int myBPM;
float temperatureC;

void setup() {   

  Serial.begin(115200);          // For Serial Monitor
  pinMode(led,OUTPUT);
  pinMode(buzzer,OUTPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer.
  display.clearDisplay();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(32,0);
  display.println("Initializing...");
  display.display();

  delay(1000);
  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");//This prints one time at Arduino power-up,  or on Arduino reset.  
  }
  /*ser.begin(9600);
  ser.println("AT");
  delay(1000);
  ser.println("AT+GMR");
  delay(1000);
  ser.println("AT+CWMODE=3");
  delay(1000);
  ser.println("AT+RST");
  delay(5000);
  ser.println("AT+CIPMUX=1");
  delay(1000);
  
  String cmd="AT+CWJAP=\"Alexahome\",\"98765432\"";
  ser.println(cmd);
  delay(1000);
  ser.println("AT+CIFSR");
  delay(1000);*/
}



void loop() {

    display.setCursor(64,8);
    display.setTextSize(2);
    display.write(3);
    display.display();
  int reading = analogRead(tempPin);

  // Convert that reading into voltage
  float voltage = reading * (4.5 / 1024.0);

  // Convert the voltage into the temperature in Celsius
  temperatureC = voltage * 100 - 12;

  // Print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.print("\xC2\xB0"); // shows degree symbol
  Serial.print("C  |  ");
  
  // Print the temperature in Fahrenheit
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  Serial.print(temperatureF);
  Serial.print("\xC2\xB0"); // shows degree symbol
  Serial.println("F");


  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened".
    myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
    Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
    Serial.print("BPM: ");                        // Print phrase "BPM: " 
    Serial.println(myBPM); // Print the value inside of myBPM.
    display.clearDisplay();
    display.setCursor(0,12);
    display.println();
    display.setTextSize(0.75);
    display.println("A HeartBeat Happened!");
    display.setTextSize(1.75);
    display.print("BPM: ");
    //display.setTextSize(1);
    display.println(myBPM);
    display.print("Temp: ");
    display.print(temperatureC);
    display.println(" C");
    display.display();
    if(myBPM<50 || myBPM>150)
    {
      digitalWrite(led,HIGH);
      digitalWrite(buzzer,HIGH);
      delay(2000);
      digitalWrite(led,LOW);
      digitalWrite(buzzer,LOW);
    }
  }
  else{
    display.clearDisplay();
    display.setTextSize(1.2);                    
    display.setTextColor(WHITE);             
    display.setCursor(32,20);                
    display.println("Please Place "); 
    display.setCursor(32,28);
    display.println("your finger ");  
    display.display();
  }
  delay(1000);                    // considered best practice in a simple sketch.
  esp_8266();
}
void esp_8266()
{
// TCP connection AT+CIPSTART=4,"TCP","184.106.153.149",80
  String cmd = "AT+CIPSTART=4,\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  Serial.println(cmd);
  if(ser.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr +=String(temperatureC);
  getStr +="&field2=";
  getStr +=String(myBPM);
  getStr += "\r\n\r\n";
  // send data length
  cmd = "AT+CIPSEND=4,";
  cmd += String(getStr.length());
  ser.println(cmd);
  Serial.println(cmd);
  delay(1000);
  ser.print(getStr);
  Serial.println(getStr); //thingspeak needs 15 sec delay between updates
  delay(3000);
}
