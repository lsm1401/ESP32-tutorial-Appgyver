/*
KAIST Tutorial for ESP32 
*/
//include librarary header
#include <Arduino.h> 
#include "NeuralNetwork.h" //include model header
#include <HTTPClient.h> // #include "WiFi.h"//dependency of HTTPClient.h
#include "ArduinoJson.h"

#include <NTPClient.h>                        //UPDATE--for Appgyver tutorial
#include <WiFiUdp.h>                          //UPDATE--for Appgyver tutorial

// Define NTP Client to get time
WiFiUDP ntpUDP; // UDP client                 //UPDATE--for Appgyver tutorial
NTPClient timeClient(ntpUDP); // NTP client   //UPDATE--for Appgyver tutorial

//Select Camera model 
#define CAMERA_MODEL_ESP_EYE

#include "esp_camera.h"
#include "camera_pins.h"
#include "img_converters.h"

// WiFi credentials.
const char* ssid = "TP-Link_29F3"; //Update Wifi Network ID  
const char* password = "93374262"; //Update Wifi Password

//Your Domain name with URL path or IP address with path
// const char* eventmeshEndpoint = "https://kaistdevicecap-sap-trainer03-sumin.fe56704.kyma.ondemand.com/service/kaistdevice/Objectdetection";                    //UPDATE--for Appgyver tutorial

// Messaging for AppGyver
const char* eventmeshEndpoint = "https://kaistcpapp-kaist-bridge.fe56704.kyma.ondemand.com/service/kaistbridgecatalog/KAIST_BRIDGE/TEAM_2"; //update team's URL: //UPDATE--for Appgyver tutorial


// Define Framesize
#define FRAME_SIZE FRAMESIZE_96X96
#define WIDTH 96
#define HEIGHT 96

// Define Image array
uint8_t img_array[HEIGHT * WIDTH] = { 0 };   // grayscale goes from 0 to 255. 

// Variables to save date and time
String formattedDate;
// String dayStamp;
// String timeStamp;

// Camera setup  
bool setup_camera(framesize_t);
void frame_to_array(camera_fb_t * frame);
void print_image_shape(camera_fb_t * frame);
bool capture_image();

// Define Model 
NeuralNetwork *nn;

//Send data 
 //UPDATE--for Appgyver tutorial
// Reference  - https://statics.teams.cdn.office.net/evergreen-assets/safelinks/1/atp-safelinks.html
void send_data(String object_detected, float probability, String device){
  HTTPClient http;
  StaticJsonDocument<256> doc;
  JsonObject root = doc.to<JsonObject>();
  // root["TEAM_NAME"] = device;                  
  root["LAST_UPDATED"] = formattedDate;           
  root["RESULT"] = object_detected;               
  root["IS_READ"] = false;                        
  // root["object_detected"] = object_detected;   
  // root["probability"] = probability;           
  // root["device"] = device;                     
  

  serializeJsonPretty(root, Serial);
  String json;
  serializeJson(root,json) ; 

  http.begin(eventmeshEndpoint);
  http.addHeader("Content-Type", "application/json");
  // http.addHeader("Host","kaistdevicecap-sap-trainer03-sumin.fe56704.kyma.ondemand.com");
  http.addHeader("Host","kaistcpapp-kaist-bridge.fe56704.kyma.ondemand.com");

  
  // int httpResponseCode = http.POST(json);      
  int httpResponseCode = http.PUT(json);         

  Serial.println(httpResponseCode); 
  
}
 //UPDATE--for Appgyver tutorial


void setup() {
  // Begin setup 
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  // Load Model 
  nn = new NeuralNetwork();
  Serial.println("tf model loaded!"); 
  
  // Setup camera 
  Serial.println("camera setup"); 
  Serial.println(setup_camera(FRAME_SIZE) ? "OK" : "ERR INIT");
  
  // Connect to Wifi.
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.println(WiFi.localIP());

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(3600);
}

void loop() {

//* UPDATE--for Appgyver tutorial
  while(!timeClient.update()) {                     
    timeClient.forceUpdate();                     
  }                                                
  // We need to extract date and time               
  formattedDate = timeClient.getFormattedDate();   
  Serial.println(formattedDate);      
  //* UPDATE--for Appgyver tutorial

  // put your main code here, to run repeatedly:
  if (!capture_image()) {
        Serial.println("Failed capture");
        delay(2000);
      return;
  }
  if (capture_image){
      Serial.println("CAMERA OK");

      for (uint32_t i = 0; i < HEIGHT*WIDTH; i++){
        nn->getInputBuffer()[i] = (float)(img_array[i])/255;
      }

      float probability = nn->predict(); 

      Serial.print("Object_detected:"); 

      String object_detected; 
      String device = "SAP-trainer03"; // Update Team name 
      if (probability>0.5){
        Serial.print("Apple"); 
        object_detected = "Apple"; 
      }
      else{
        Serial.print("Lemon"); 
        object_detected = "Lemon"; 
        probability = 1.00 - probability; 
      }
      Serial.println(); 
      Serial.print("Pred:"); 
      Serial.print(probability);
      Serial.println(); 

      //send data
      send_data(object_detected, probability, device); 

      delay(30000);

  }
}

// Setup camera 
bool setup_camera(framesize_t frameSize) {
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
    config.pixel_format =  PIXFORMAT_GRAYSCALE; 
    config.frame_size = frameSize;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    bool ok = esp_camera_init(&config) == ESP_OK;

    //Changing ESP32-CAM Camera Settings Arduino Sketch 
    sensor_t *sensor = esp_camera_sensor_get();
    
    // sensor->set_brightness(sensor,1);  //up the brightness just a bit  // -2 to 2
    // sensor->set_contrast(sensor, -2);       // -2 to 2
    // sensor->set_saturation(sensor, 1); // lower the saturation //-2 to 2 
    // sensor->set_vflip(sensor, 1);          // 0 = disable , 1 = enable 
    // sensor->set_special_effect(sensor, 2); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    sensor->set_awb_gain(sensor, 1);       // 0 = disable , 1 = enable
    sensor->set_wb_mode(sensor, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    sensor->set_framesize(sensor, frameSize);

    return ok;
}

bool capture_image() {

    camera_fb_t * frame = NULL;
    frame = esp_camera_fb_get();
    const char *data = (const char *)frame->buf;
    print_image_shape(frame);
    frame_to_array(frame);
    esp_camera_fb_return(frame);

    if (!frame)
        return false;

    return true;
  }


void print_image_shape(camera_fb_t * frame){

     // print shape of image and total length (=heigth*width)
     Serial.print("Width: ");
     Serial.print(frame->width);
     Serial.print("\tHeigth: ");
     Serial.print(frame->height);
     Serial.print("\tLength: ");
     Serial.println(frame->len);

 }

 void frame_to_array(camera_fb_t * frame){

    int len = frame->len;
    char imgBuffer[frame->len];
    int counter = 0;
    int h_counter = 0;
    int w_counter = 0;

     // write values from buffer into 2D Array
     for (int h=0; h < HEIGHT; h++){
        //  Serial.println(h);
         for (int w=0; w < WIDTH; w++){
            //  Serial.println(w);
            int position = h*(len/HEIGHT)+w;
            // Serial.print(position);
            // Serial.print(",");
            img_array[h*w] = {frame->buf[position]};
            // Serial.print(img_array[h*w]);
            // Serial.println();
            //  delay(2);
         }
        // delay(100); 
     }
    Serial.println("=====================");

 }

