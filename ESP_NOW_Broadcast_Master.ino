/*
    ESP-NOW Broadcast Master
    Lucas Saavedra Vaz - 2024

    This sketch demonstrates how to broadcast messages to all devices within the ESP-NOW network.
    This example is intended to be used with the ESP-NOW Broadcast Slave example.

    The master device will broadcast a message every 5 seconds to all devices within the network.
    This will be done by registering a peer object with the broadcast address.

    The slave devices will receive the broadcasted messages and print them to the Serial Monitor.
*/

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

/* Definitions */

#define ESPNOW_WIFI_CHANNEL 6

/*-----------------SCALABLE-------------------*/

// CHANGE THIS NUMBER WHEN SCALING UP
#define NUM_SLAVES 2

/*-------------------------------------------*/

// Every broadcast contains a slave_id so each slave knows whether to act on it.
// The 'message' field holds the unique string for that slave.

/*---------CUSTOMIZE--------*/

struct SlaveMessage {
  uint8_t slave_id;   // Which slave this message is for (0-indexed)
  char message[31];   // The unique string for that slave (31 bytes + 1 for slave_id = 32 total)
};

/*--------------------------*/

/* Classes */

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor — called when you create the object, sets up the peer using the
  // special broadcast address (FF:FF:FF:FF:FF:FF), which means "send to everyone"
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Initializes ESP-NOW and registers this peer so the radio knows about it.
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network - sends a SlaveMessage struct instead of a plain string
  bool send_message(const SlaveMessage *msg) {
    if (!send((uint8_t *)msg, sizeof(SlaveMessage))) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

/* Global Variables */

uint32_t msg_count = 0;

// Create a broadcast peer object
// broadcast_peer is the actual peer object created from the class above
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr);

/*-----------------SCALABLE-------------------*/

// Define the unique strings for each slave.
// ADD MORE ENTRIES WHEN SCALING UP
const char *slave_strings[NUM_SLAVES] = {
  "Hello from master, Slave 0!",
  "Hey there, Slave 1, different data!"
};

/*-------------------------------------------*/

/* Main */

void setup() {
  Serial.begin(115200);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.printf("ESP-NOW version: %d, max data length: %d\n", ESP_NOW.getVersion(), ESP_NOW.getMaxDataLen());

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
}

// THIS IS WHERE THE DATA GETS SENT
void loop() {
  // Loop through each slave and send it a unique message.
  // To scale to 35 slaves, just increase NUM_SLAVES and add strings to slave_strings[].
  for (uint8_t i = 0; i < NUM_SLAVES; i++) {
    SlaveMessage msg;
    msg.slave_id = i;
    snprintf(msg.message, sizeof(msg.message), "%s #%lu", slave_strings[i], msg_count);

    Serial.printf("Sending to slave %d: %s\n", i, msg.message);

    if (!broadcast_peer.send_message(&msg)) {
      Serial.printf("Failed to send message to slave %d\n", i);
    }

    // Small delay between sends to avoid flooding the channel
    delay(20);
  }

  msg_count++;
  delay(5000);
}
