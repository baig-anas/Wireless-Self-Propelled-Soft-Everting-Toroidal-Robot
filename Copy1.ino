#include <WiFiNINA.h>
#include <MKRMotorCarrier.h>

//#include <ArduinoMotorCarrier.h>
#include <utility/wifi_drv.h> //


char ssid[] = " ";             //  network SSID (name) between the " "
char pass[] = " ";      // network password between the " "
//int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket 

WiFiClient client = server.available();

int duty = 30; // duty cycle 30, 9V*0.3
int Acceleration = 2;
int Duration = 5000;



static float batteryVoltage; //Variable to store the battery voltage
static float minBattery; //low battery limit (discharged) 
static float maxBattery; //low battery limit (discharged) 
void setup() {
  Serial.begin(9600);
  WiFiDrv::pinMode(26, OUTPUT); //define green pin
  WiFiDrv::pinMode(25, OUTPUT); //define red pin


//wifi settings begin, establishing a connection with the ssid
  //while (!Serial);
  
  enable_WiFi();
  connect_WiFi();

  server.begin();
  printWifiStatus();
//wifi settings end
// motor settings, establishing a connection with the MKR board
  if (controller.begin())
  {
    Serial.print("MKR Motor Shield connected, firmware version ");
    Serial.println(controller.getFWVersion());
  }
  else
  {
    Serial.println("Couldn't connect! Is the red led blinking? You may need to update the firmware with FWUpdater sketch");
    while(1);
  }
  Serial.println("reboot");
  controller.reboot();
  delay(500);
  M3.setDuty(0);
  M4.setDuty(0);
// end of motor settings

float batteryVoltage = (float)battery.getRaw() / 77;

minBattery = 6.0;
maxBattery = 11.0;
Serial.print("Battery voltage: ");
Serial.println(batteryVoltage);
}

void loop() {



delay(5000);
  client = server.available();

  if (client) {
    printWEB();
  }
  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  //long rssi = WiFi.RSSI();
  //Serial.print("signal strength (RSSI):");
  //Serial.print(rssi);
  //Serial.println(" dBm");

  Serial.print("To control the motor go to http://");
  Serial.println(ip);
}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    WiFiDrv::analogWrite(25, 255); //to indicate when it is trying to connect
    delay(200);
    WiFiDrv::analogWrite(25, 0);
    // wait 10 seconds for connection:
    delay(10000);
  }
WiFiDrv::analogWrite(26, 255); //to indicate when it is connected
delay(200);
WiFiDrv::analogWrite(26, 0);
}

void printWEB() {

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");

            client.println();

            client.println("<HTML>");
            client.println("<HEAD>");
            client.println("<TITLE>Arduino GET test page</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");

            client.println("<H1>Robot Control</H1>");

            client.println("<input type=submit value=Forward style=width:100px;height:50px onClick=location.href='/F\'>");
            client.print("<input type=submit value=Back style=width:100px;height:50px onClick=location.href='/B\'>");
            client.print("<input type=submit value=Accelerate style=width:100px;height:50px onClick=location.href='/A\'>");
            client.print("<input type=submit value=Stop style=width:100px;height:50px onClick=location.href='/S\'> <br>");
            client.print(" <br><br>");
            client.print("Light: ");
            client.print("<input type=submit value=On style=width:100px;height:45px onClick=location.href='/ON\'>");
            client.print("<input type=submit value=OFF style=width:100px;height:45px onClick=location.href='/OFF\'> <b>");
            client.print(" <br><br>");
            client.print("Battery: ");
            client.println(batteryVoltage);
            client.println("V");
            client.print("<input type=submit value=Battery style=width:100px;height:45px onClick=location.href='/V\'> <br>");
            
            
            client.println("</BODY>");
            client.println("</HTML>");
 
            delay(1);
          //stopping client
            client.stop();

            // The HTTP response ends with another blank line:
      client.println();
      // break out of the while loop:
      break;
    }
    else {      // if you got a newline, then clear currentLine:
      currentLine = "";
    }
  }
  else if (c != '\r') {    // if you got anything else but a carriage return character,
    currentLine += c;      // add it to the end of the currentLine
  }

//===========

batteryVoltage = (float)battery.getRaw() / 77;

if (batteryVoltage < minBattery) {
Serial.println(" ");
Serial.println("Warning: Low battery!");
Serial.print("Battery voltage: ");
Serial.println(batteryVoltage);
M1.setDuty(0);
M2.setDuty(0);
M3.setDuty(0);
M4.setDuty(0);
while (batteryVoltage < minBattery){
  batteryVoltage = (float)battery.getRaw() / 77;
  controller.ping();
  delay(100);

}
}
else if (batteryVoltage > maxBattery){
M1.setDuty(0);
M2.setDuty(0);
M3.setDuty(0);
M4.setDuty(0);
while (batteryVoltage > maxBattery){
  batteryVoltage = (float)battery.getRaw() / 77;
  controller.ping();
  delay(100);
}
}
else {
if (currentLine.endsWith("GET /F")) {

M3.setDuty(duty);
M4.setDuty(-duty);
delay(500);

controller.ping();     
}
else if (currentLine.endsWith("GET /B")) {

M3.setDuty(-duty);
M4.setDuty(duty);
delay(500);
controller.ping();     
}
else if (currentLine.endsWith("GET /S")) {

M3.setDuty(duty*0);
M4.setDuty(duty*0);
delay(500);
controller.ping();        
}
else if (currentLine.endsWith("GET /A")) {

M3.setDuty(duty*Acceleration);
M4.setDuty(-duty*Acceleration);
delay(Duration);
M3.setDuty(duty);
M4.setDuty(-duty);
controller.ping();           
}
else if (currentLine.endsWith("GET /ON")) {
  WiFiDrv::analogWrite(27, 255);
  //delay(Duration*2);
  //WiFiDrv::analogWrite(27, 0); //turns off automatically
}
else if (currentLine.endsWith("GET /OFF")){
  WiFiDrv::analogWrite(27, 0);
}
    
else if (currentLine.endsWith("GET /V")){
  client = server.available();
  if (client){
    printWEB();
  }
}
}
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
 
}