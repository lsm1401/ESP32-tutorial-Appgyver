# ESP32-tutorial-Appgyver
ESP32-tutorial-Appgyver
This is adjustment tutorial of ESP32-tutorial and updates as follows: 
## main.cpp
- Including Libraries <NTPClient.h> , <WiFiUdp.h> : to get timestamp dataset from wifi
- Message for AppGyver URL link: https://kaistcpapp-kaist-bridge.fe56704.kyma.ondemand.com/service/kaistbridgecatalog/KAIST_BRIDGE/TEAM_2
- REST API from POST to PUT : http.PUT(json) 
- send_data with "LAST_UPDATED", "RESULT" and "IS_READ"
## .pio/libdeps/esp32dev/NTPClient  
- NTPClient.cpp and NTPClient.h

## Reference 
- https://github.com/arduino-libraries/NTPClient/blob/master/NTPClient.h
- https://github.com/taranais/NTPClient/blob/master/NTPClient.cpp  (for getFormattedDate)
- https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
