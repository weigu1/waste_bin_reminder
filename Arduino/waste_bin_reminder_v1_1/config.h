/****** WiFi SSID and PASSWORD ******/
const char *MY_WIFI_SSID = "your_ssid";
const char *MY_WIFI_PASSWORD = "your_password";

/****** WiFi and network settings ******/
// UDP logging settings if enabled in setup(); Port used for UDP logging
const word UDP_LOG_PORT = 5558;
// IP address of the computer receiving UDP log messages
const byte UDP_LOG_PC_IP_BYTES[4] = {192, 168, 178, 100};
// optional (access with UDP_logger.local)
const char *NET_MDNSNAME = "ESP_MQTT";
// optional hostname
const char *NET_HOSTNAME = "ESP_MQTT";
// only if you use a static address (uncomment //#define STATIC in ino file)
const byte NET_LOCAL_IP_BYTES[4] = {192, 168, 178, 155};
const byte NET_GATEWAY_BYTES[4] = {192, 168, 178, 1};
const byte NET_MASK_BYTES[4] = {255,255,255,0};
const byte NET_DNS_BYTES[4] = {8,8,8,8}; //  second dns (first = gateway), 8.8.8.8 = google
// only if you use OTA (uncomment //#define OTA in ino file)
const char *MY_OTA_NAME = "esp_mqtt"; // optional (access with esp_with_ota.local)
// Linux Create Hasgh with: echo -n 'P@ssword1' | md5sum
const char *MY_OTA_PASS_HASH = "myHash";     // Hash for password

/****** MQTT settings ******/
const char *MQTT_SERVER = "192.168.178.222";
const long PUBLISH_TIME = 30000; //Publishes every in milliseconds
const int MQTT_MAXIMUM_PACKET_SIZE = 1024; // look in setup()
const char *MQTT_CLIENT_ID = "waste_bin_reminder_23"; // this must be unique!!!
String MQTT_TOPIC_OUT = "bin_reminder/data";
String MQTT_TOPIC_IN = "bin_reminder/command";
const short MY_MQTT_PORT = 1883; // or 8883
// only if you use MQTTPASSWORD (uncomment //#define MQTTPASSWORD in ino file)
const char *MY_MQTT_USER = "me";
const char *MY_MQTT_PASS = "meagain";

const char* server_name = "https://www.weigu.lu/waste_collection/index.php";

const byte PIXEL_PIN = 7; //Lolin Pin 7, Dev Pin 8
const byte NUMPIXELS = 1;

String which_bin = "";
String json = "nothing";