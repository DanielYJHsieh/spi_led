/*
 * WeMos D1 Mini (ESP8266) SPI數據傳輸測試
 * 純SPI測試，不使用CS信號
 * 移植自Arduino UNO版本
 * 
 * 硬體連接:
 * - MOSI (D7/GPIO13) -> 邏輯分析儀 CH0
 * - SCK (D5/GPIO14) -> 邏輯分析儀 CH1  
 * - GND -> 邏輯分析儀 GND
 * 
 * LA SPI設定:
 * - MOSI: CH0, SCK: CH1
 * - 時鐘極性: CPOL=0, 時鐘相位: CPHA=0
 * 
 * ESP8266 SPI Pin腳對應:
 * - D5 (GPIO14) = SCK
 * - D6 (GPIO12) = MISO
 * - D7 (GPIO13) = MOSI
 * - D8 (GPIO15) = CS (未使用)
 */

#include <SPI.h>

// Pin 定義 (ESP8266 WeMos D1 Mini)
#define MOSI_PIN 13  // D7 (GPIO13) - SPI MOSI
#define SCK_PIN 14   // D5 (GPIO14) - SPI Clock

// SPI 設定參數
// ESP8266 SPI頻率選項 (80MHz系統時鐘):
// 可設定範圍: 1MHz ~ 40MHz
// 常用頻率: 1M, 2M, 4M, 8M, 10M, 20M, 40MHz
const uint32_t SPI_FREQUENCY = 3333333;  // 3.33MHz (精確頻率，ESP8266支援)

// LED 設定
const uint8_t NUM_LEDS = 18;  // 增加到18顆LED

// 全局變數用於儲存所有LED的RGB數據
uint8_t ledData[NUM_LEDS][3]; // [LED序號][R=0, G=1, B=2]

// 用於UART通信的緩衝區
const int MAX_INPUT_LENGTH = 20;
char inputBuffer[MAX_INPUT_LENGTH];
int bufferPosition = 0;

void setup() {
  // 啟用串列通訊用於UART輸入
  Serial.begin(115200);
  
  Serial.println("WeMos D1 Mini (ESP8266) SPI LED控制程式");
  Serial.println("輸入格式: LED,R,G,B (例如: 0,255,255,255)");
  Serial.println("LED範圍: 0-17, RGB範圍: 0-255");
  
  // 初始化SPI
  SPI.begin();
  
  // 初始化所有LED為白色 (R=255, G=255, B=255)
  for (int i = 0; i < NUM_LEDS; i++) {
    ledData[i][0] = 0xFF; // R
    ledData[i][1] = 0xFF; // G
    ledData[i][2] = 0xFF; // B
  }
  
  // 初始顯示所有LED
  updateLEDs();
}

void loop() {
  // 檢查是否有新的UART輸入
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    
    // 處理回車符或換行符，兩者都視為命令結束
    if (inChar == '\n' || inChar == '\r') {
      if (bufferPosition > 0) {  // 確保不處理空命令
        inputBuffer[bufferPosition] = '\0'; // 字符串結尾
        Serial.print("處理命令: ");
        Serial.println(inputBuffer);
        processCommand();
        bufferPosition = 0; // 重置緩衝區位置
      }
    } 
    // 添加字符到緩衝區 (避免溢出)
    else if (bufferPosition < MAX_INPUT_LENGTH - 1) {
      inputBuffer[bufferPosition++] = inChar;
    }
  }
}

// 處理接收到的命令
void processCommand() {
  int led, r, g, b;
  
  // 處理特殊命令
  if (strcmp(inputBuffer, "test") == 0) {
    Serial.println("OK");
    return;
  }
  else if (strcmp(inputBuffer, "status") == 0) {
    // 輸出當前所有LED的狀態
    Serial.println("當前LED狀態:");
    for (int i = 0; i < NUM_LEDS; i++) {
      Serial.print(i);
      Serial.print(":");
      Serial.print(ledData[i][0]);
      Serial.print(",");
      Serial.print(ledData[i][1]);
      Serial.print(",");
      Serial.print(ledData[i][2]);
      Serial.print(" ");
      
      // 每4個LED換行顯示
      if ((i+1) % 4 == 0) {
        Serial.println();
      }
    }
    Serial.println();
    return;
  }
  else if (strcmp(inputBuffer, "help") == 0) {
    Serial.println("Commands: LED,R,G,B | test | status | reset");
    return;
  }
  else if (strcmp(inputBuffer, "reset") == 0) {
    // 重設所有LED為白色
    for (int i = 0; i < NUM_LEDS; i++) {
      ledData[i][0] = 0xFF; // R
      ledData[i][1] = 0xFF; // G
      ledData[i][2] = 0xFF; // B
    }
    updateLEDs();
    Serial.println("OK");
    return;
  }
  
  // 解析常規命令格式: LED,R,G,B
  int parsed = sscanf(inputBuffer, "%d,%d,%d,%d", &led, &r, &g, &b);
  
  if (parsed == 4) {
    // 檢查值的有效範圍
    if (led >= 0 && led < NUM_LEDS && 
        r >= 0 && r <= 255 && 
        g >= 0 && g <= 255 && 
        b >= 0 && b <= 255) {
      
      // 更新LED數據
      ledData[led][0] = r;
      ledData[led][1] = g;
      ledData[led][2] = b;
      
      // 輸出確認信息
      Serial.print("設置LED ");
      Serial.print(led);
      Serial.print(" 為 RGB(");
      Serial.print(r);
      Serial.print(",");
      Serial.print(g);
      Serial.print(",");
      Serial.print(b);
      Serial.println(")");
      
      // 更新所有LED顯示
      updateLEDs();
    } else {
      Serial.println("ERR: 參數超出範圍");
    }
  } else {
    Serial.println("ERR: 格式不正確");
  }
}

// 更新所有LED顯示
void updateLEDs() {
  // 計算所需的緩衝區大小 (每個LED的每個顏色通道需要4個字節)
  const int bufferSize = NUM_LEDS * 3 * 4; // 18 LEDs * 3 channels * 4 bytes
  uint8_t dataBuffer[bufferSize];
  
  int bufferIndex = 0;
  
  // 為每個LED生成數據
  for (int i = 0; i < NUM_LEDS; i++) {
    // 為每個LED處理RGB三個通道
    for (int color = 0; color < 3; color++) {
      uint32_t encoded = convertTo32bit(ledData[i][color]);
      
      // 將32位編碼拆分為4個字節
      dataBuffer[bufferIndex++] = (encoded >> 24) & 0xFF;
      dataBuffer[bufferIndex++] = (encoded >> 16) & 0xFF;
      dataBuffer[bufferIndex++] = (encoded >> 8) & 0xFF;
      dataBuffer[bufferIndex++] = encoded & 0xFF;
    }
  }
  
  // 開始SPI傳輸
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  SPI.writeBytes(dataBuffer, bufferSize);
  SPI.endTransaction();
  
  Serial.println("LED顯示已更新");
}

/*
 * 8位轉換為32位 (參考aRGB LED協議)
 * Logic 0 -> 四個位元: 1000 (300ns High + 900ns Low)
 * Logic 1 -> 四個位元: 1110 (900ns High + 300ns Low)
 */
uint32_t convertTo32bit(uint8_t data) {
  uint32_t result = 0;
  
  // 從MSB到LSB處理每一位
  for (int i = 7; i >= 0; i--) {
    if ((data >> i) & 0x01) {
      // Logic 1: 1110 (binary) = 0xE (hex)
      result = (result << 4) | 0xE;
    } else {
      // Logic 0: 1000 (binary) = 0x8 (hex)
      result = (result << 4) | 0x8;
    }
  }
  
  return result;
}

/*
 * 發送32位數據 (使用writeBytes一次傳輸)
 */
void send32bitData(uint32_t data) {
  // 將32位數據分解為4個位元組陣列
  uint8_t bytes[4] = {
    (data >> 24) & 0xFF,  // MSB
    (data >> 16) & 0xFF,
    (data >> 8) & 0xFF,
    data & 0xFF           // LSB
  };
  
  // 一次傳輸4個位元組
  SPI.writeBytes(bytes, 4);
}

/*
 * 發送8位數據並轉換為32位編碼
 */
void send8bitAsEncoded(uint8_t data) {
  uint32_t encoded = convertTo32bit(data);
  send32bitData(encoded);
}
