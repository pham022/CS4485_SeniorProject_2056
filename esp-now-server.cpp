#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> // Include esp_wifi.h for channel configuration

// Set the communication channel
#define CHANNEL 1

uint8_t receiverMAC[] = {0x88, 0x13, 0xBF, 0x07, 0xAD, 0xC0}; // Reverse order of MAC displayed in the client output

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  // If delivery fails, reinitialize the peer
  if (status != ESP_NOW_SEND_SUCCESS) {
    esp_now_del_peer(mac_addr);  // Remove the peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = CHANNEL;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to re-add peer");
    } else {
      Serial.println("Peer re-added successfully");
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE); 
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
  esp_now_register_send_cb(OnDataSent);

  // Set up peer information with cleared struct and required fields
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t)); // Clear the structure
  memcpy(peerInfo.peer_addr, receiverMAC, 6);         // Set MAC address
  peerInfo.channel = CHANNEL;                         // Set channel
  peerInfo.ifidx = WIFI_IF_STA;                       // Set interface
  peerInfo.encrypt = false;                           // No encryption

if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  Serial.println("Failed to add peer");
} else {
  Serial.println("Peer added successfully");
}

  Serial.println("ESP-NOW Server set up on Channel 1 and ready to send data.");
}

void loop() {
  int number = 2;
  float coordinates[2] = {35.1589, -112.45698};
  bool isMarkerFound = true;
  bool moveUp = true;

  uint8_t dataToSend[sizeof(number) + sizeof(coordinates) + sizeof(isMarkerFound) + sizeof(moveUp)];

  memcpy(dataToSend, &number, sizeof(number));
  memcpy(dataToSend + sizeof(number), coordinates, sizeof(coordinates));
  memcpy(dataToSend + sizeof(number) + sizeof(coordinates), &isMarkerFound, sizeof(isMarkerFound));
  memcpy(dataToSend + sizeof(number) + sizeof(coordinates) + sizeof(isMarkerFound), &moveUp, sizeof(moveUp));

// Declare result only once, outside the if-else block
esp_err_t result = esp_now_send(receiverMAC, dataToSend, sizeof(dataToSend));

if (result == ESP_OK) {
  Serial.println("Data sent successfully");
} else {
  Serial.print("Error sending data: ");
  switch (result) {
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP-NOW not initialized");
      break;
    case ESP_ERR_WIFI_NOT_INIT:
      Serial.println("Wi-Fi not initialized");
      break;
    case ESP_ERR_ESPNOW_ARG:
      Serial.println("Invalid argument");
      break;
    case ESP_ERR_ESPNOW_INTERNAL:
      Serial.println("Internal error");
      break;
    case ESP_ERR_ESPNOW_NO_MEM:
      Serial.println("Out of memory");
      break;
    case ESP_ERR_ESPNOW_NOT_FOUND:
      Serial.println("Peer not found");
      break;
    case ESP_ERR_ESPNOW_IF:
      Serial.println("Interface error");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}
  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }

  delay(1000); // Reduced delay to avoid blocking
}
