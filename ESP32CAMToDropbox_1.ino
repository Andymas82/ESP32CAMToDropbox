#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_camera.h>

#define Y2_GPIO_NUM    5   // D0
#define Y3_GPIO_NUM    18  // D1
#define Y4_GPIO_NUM    19  // D2
#define Y5_GPIO_NUM    21  // D3
#define Y6_GPIO_NUM    36  // D4
#define Y7_GPIO_NUM    39  // D5
#define Y8_GPIO_NUM    34  // D6
#define Y9_GPIO_NUM    35  // D7
#define XCLK_GPIO_NUM  0   // XCLK
#define PCLK_GPIO_NUM  22  // PCLK
#define VSYNC_GPIO_NUM 25  // VSYNC
#define HREF_GPIO_NUM  23  // HREF
#define SIOD_GPIO_NUM  26  // SDA
#define SIOC_GPIO_NUM  27  // SCL
#define PWDN_GPIO_NUM  32  // PWDN
#define RESET_GPIO_NUM 33  // RESET

const char* ssid = "Your WIFI SSID";
const char* password = "Your WIFI PASS";
const char* dropboxAccessToken = "Your Dropbox Access Token"; 

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // CAM
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // CAM START
  esp_camera_init(&config);
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure(); 

    Serial.println("Starting connection to Dropbox...");

    if (http.begin(client, "https://content.dropboxapi.com/2/files/upload")) { 
        Serial.println("Connected to Dropbox API.");

        // Path
         String apiArg = "{\"path\": \"YOUR-APP-PATH/image.jpg\", \"mode\": \"overwrite\", \"autorename\": true, \"mute\": false, \"strict_conflict\": false}";
        http.addHeader("Authorization", String("Bearer ") + dropboxAccessToken);
        http.addHeader("Dropbox-API-Arg", apiArg);
        http.addHeader("Content-Type", "application/octet-stream");
        http.addHeader("Content-Length", String(fb->len)); 

        // POST-request
        int httpResponseCode = http.POST(fb->buf, fb->len);

        // response
        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Server response: " + response);
        } else {
            Serial.printf("Error code: %d\n", httpResponseCode);
            String errorResponse = http.getString();
            Serial.println("Error response: " + errorResponse);
        }

        http.end();
    } else {
        Serial.println("Failed to connect to Dropbox API.");
    }
  } else {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }

  esp_camera_fb_return(fb);
  delay(500); // Delay 
}
