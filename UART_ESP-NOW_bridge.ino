/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
 
#include "esp_now.h"
#include "WiFi.h"

// ЗАМЕНИТЕ МАС-АДРЕСОМ ПЛАТЫ-ПОЛУЧАТЕЛЯ
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xF0, 0x2D, 0x3C};//board#1
//uint8_t broadcastAddress[] = {0x0C, 0xB8, 0x15, 0xEC, 0xA5, 0x8C};//board#2
 
char c_tx[128];//received message from uart. for send via wifi
char c_rx[128];//received from wifi
int c_tx_counter=0; //couner of recived bytes from UART
char temp[1]; // Временная переменная для принятого байта из UART.
const int c_tx_counter_max=127; //max number of element of UART buffer

const char control_char='\n'; //UART control char (end of command). may be \n or \r
 
// Обратная функция отправки по esp-now //debug
//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("    Last Packet Send Status:");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "   Delivery Success" : "   Delivery Fail");
//}

// Обратная функция при получении по esp-now
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&c_rx, incomingData, len);
  c_rx[len]='\0';
  //Serial.print("Received message: ");//debug
  Serial.print(c_rx);
}

 
void setup() {
  Serial.begin(115200);
 
  // Выбираем режим WiFi
  WiFi.mode(WIFI_STA);
 
  // Запускаем протокол ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  // Регистрируем обратные функции
  //esp_now_register_send_cb(OnDataSent);//debug
  esp_now_register_recv_cb(OnDataRecv);
  
  // Указываем получателя
  esp_now_peer_info_t peerInfo; //попробовать добавить {}. Т.к. в другом проекте была ошибка из-за этого. 13.12.2024
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void send_message(int len,char* message){  
//  Serial.print("message=");//debug
//  Serial.print(message);
//  Serial.print("len=");
//  Serial.print(len);
  
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, len);
   
//  if (result == ESP_OK) {//debug
//    Serial.print("   Sent with success");
//  }
//  else {
//    Serial.print("   Error sending the data");
//  }
}
 
void loop() {

  if(Serial.available()>0){    
    Serial.readBytes(temp, 1); // Читаем байт из UART.
    c_tx[c_tx_counter] = temp[0]; // Добавляем его в буфер.  
    c_tx_counter++;
  }
  if(temp[0]=='\n' or temp[0]=='\r' or c_tx_counter>=c_tx_counter_max){
    c_tx[c_tx_counter]='\0';
    send_message(c_tx_counter,c_tx);
    c_tx_counter=0;
    temp[0]=NULL;
  }
  


}
