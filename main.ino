// Imports
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

// Global WiFi variables
const char* ssid     = "FRITZ!Box 7560 UG"; // Your ssid
const char* password = "29588493858099152795"; // Your Password

// Global temperature variables
float temp_celsius = 0;
// Number of values taken for averaging
const int num_average = 10;
float array_readings[num_average];
int counter_next_reading = 0;
boolean enough_readings = false;
const float callibration_var = -1.25;
// Create server entity
ESP8266WebServer server(80);

// Setup functions: Start serial connection, establish WiFi-connection
void setup() {
    // Serial begin with Baudrate
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Establishing connection to WiFi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    // Start connection process
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        // Print until connection established
        Serial.print(".");
    }
    // Lines printed when connection established
    Serial.println("");
    Serial.println("WiFi is connected");
    server.begin();
    Serial.println("Server started");
    Serial.println(WiFi.localIP());

    //Start mDNS
    if (!MDNS.begin("esp8266_1")) {             // Start the mDNS responder for esp8266.local; http://esp8266.local/
        Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("mDNS responder started as esp8266_1");
    MDNS.addService("http", "tcp", 80);

    // handler functions
    server.on("/val", handleTempVal);       // just the float as String in text/plain
    server.on("/", handleRoot);                 // actually looks like a webpage

    //Start server
    server.begin();
    Serial.println("HTTP server started");
}

// Loop function: Read the temperature value and process data, handle get-requests from clients
void loop() {
    // Read sensor-value and process to temperature value in degree Celsius and print to Serial
    array_readings[counter_next_reading] = (analogRead(A0) * 330.0) / 1023.0;
    Serial.print("  Temperature = ");
    Serial.print(temp_celsius);
    Serial.println(" Celsius, ");
    // Print array values to Serial port to determine value fluctuation
    Serial.print("Array vals:");
    for(int index = 0; index<num_average; index++){
        Serial.print(" ");
        Serial.print(array_readings[index]);
    }
    Serial.println();

    // Set counter
    if(counter_next_reading<num_average-1){
        counter_next_reading+=1;
    }
    else{
        counter_next_reading=0;
        enough_readings = true;
    }
    // if enough values measured, calculate average temperature
    if(enough_readings){
        float avg_temp_var = 0;
        for(int index = 0; index<num_average; index++){
            avg_temp_var += array_readings[index];
        }
        temp_celsius = (avg_temp_var/num_average) + callibration_var;
    }
    
    // handleIncoming clients
    MDNS.update();
    if(temp_celsius){
        server.handleClient();
    }
    delay(500);
}

// handle for "/" (Root)
void handleRoot() {
    server.send(200, "text/html", create_root_string());
}

// handle for "/tempVal"
void handleTempVal(){
    server.send(200, "text/plain", String(temp_celsius));
}

// create the htmlString for the Roothandler
String create_root_string(){   
    String str = "<!DOCTYPE HTML>";
    str += "<html>";
    str += "<p style='text-align: center;'><span style='font-size: x-large;'><strong>Digital Thermometer</strong></span></p>";
    str += "<p style='text-align: center;'><span style='color: #0000ff;'><strong style='font-size: large;'>Temperature (*C)= ";
    str += temp_celsius;
    str += "</strong></span></p>";
    str += "</html>";
    return str;
}