#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> // Include esp_wifi.h for channel configuration

unsigned long timeout = 30000;
unsigned long lastReceiveTime = 0;
bool reminderSent = false;

int packageNumber;
float coordinates[2];
bool isMarkerFound;
bool moveUp;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (data_len != sizeof(packageNumber) + sizeof(coordinates) + sizeof(isMarkerFound) + sizeof(moveUp)) {
    Serial.println("Received unexpected data length");
    return;
  }

  lastReceiveTime = millis();
  reminderSent = false;

  // Copy data into the variables
  memcpy(&packageNumber, data, sizeof(packageNumber));
  memcpy(&coordinates, data + sizeof(packageNumber), sizeof(coordinates));
  memcpy(&isMarkerFound, data + sizeof(packageNumber) + sizeof(coordinates), sizeof(isMarkerFound));
  memcpy(&moveUp, data + sizeof(packageNumber) + sizeof(coordinates) + sizeof(isMarkerFound), sizeof(moveUp));

  Serial.print("Package number: ");
  Serial.println(packageNumber);

  Serial.print("Coordinates: [");
  Serial.print(coordinates[0], 6);
  Serial.print(", ");
  Serial.print(coordinates[1], 6);
  Serial.println("]");

  Serial.print("isMarkerFound: ");
  Serial.println(isMarkerFound ? "true" : "false");

  Serial.print("moveUp: ");
  Serial.println(moveUp ? "true" : "false");
}

void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); 
  Serial.print("Current channel: ");
  uint8_t primary_channel;
  wifi_second_chan_t second_channel;
  esp_wifi_get_channel(&primary_channel, &second_channel);
  Serial.print("Current primary channel: ");
  Serial.println(primary_channel);
  Serial.print("Current secondary channel: ");
  Serial.println(second_channel == WIFI_SECOND_CHAN_NONE ? "None" : "Secondary");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  uint8_t pmk[16] = {0}; // Use a zeroed-out key for no encryption
  esp_now_set_pmk(pmk);
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Client set up and waiting for data.");
  lastReceiveTime = millis(); // Initialize last receive time
}

void loop() {
  unsigned long currentTime = millis();
  if ((currentTime - lastReceiveTime >= timeout / 2) && !reminderSent) {
    Serial.println("Client: Waiting for data from server...");
    reminderSent = true;  
  }

  if (currentTime - lastReceiveTime >= timeout) {
    Serial.println("Timeout reached, no data received from server");
    lastReceiveTime = currentTime;  // Reset to prevent repeated timeout messages
    reminderSent = false;           // Reset reminder flag after timeout
  }

  delay(1000); // Reduced delay to improve responsiveness
}
