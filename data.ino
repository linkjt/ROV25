#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Network configuration (adjust as needed)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177); // Arduino's IP
IPAddress serverIP(192, 168, 1, 100); // Raspberry Pi's IP
unsigned int serverPort = 5005;
unsigned int localPort = 8888; // Port Arduino will listen on (optional for sending)

EthernetUDP udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // buffer to hold incoming packets

void setup() {
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  udp.begin(localPort);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Example: Sending a sensor reading every second
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 1000) {
    lastTime = millis();
    int sensorValue = analogRead(A0);
    String data = "Sensor Value: " + String(sensorValue);
    udp.beginPacket(serverIP, serverPort);
    udp.write(data.c_str());
    udp.endPacket();
    Serial.print("Sent: ");
    Serial.println(data);
  }
  delay(10);
}
