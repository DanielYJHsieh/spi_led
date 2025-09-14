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

void setup() {
  // === 串列通訊可能影響SPI時鐘穩定性 ===
  // Serial.begin(115200);  // 串列初始化可能影響時鐘
  // Serial.println("WeMos D1 Mini (ESP8266) SPI測試程式");
  // Serial.println("測試SPI基本傳輸功能");
  
  // 禁用中斷以獲得最穩定的SPI時鐘
  // noInterrupts();  // 可選：完全禁用中斷
  
  // 初始化SPI
  SPI.begin();
  
  // === 註解掉可能造成時鐘干擾的串列輸出 ===
  // Serial.println("SPI初始化完成");
  // Serial.println("MOSI: D7 (GPIO13), SCK: D5 (GPIO14)");
  // Serial.println("SPI頻率: 3.33MHz (ESP8266精確支援)");
  // Serial.println("LA設定: CH0=MOSI, CH1=SCK");
  // Serial.println("注意: 標準SPI中MOSI空閒時為高電平");
  // Serial.println();
}

void loop() {
  static unsigned long lastTransmission = 0;
  static int transmissionStep = 0;
  
  unsigned long currentTime = millis();
  
  // 每5秒重新開始整個循環
  if (currentTime - lastTransmission >= 5000) {
    transmissionStep = 0;
    lastTransmission = currentTime;
  }
  
  // 根據步驟進行不同的傳輸，每1秒一次
  if (transmissionStep == 0 || 
      (transmissionStep > 0 && currentTime - lastTransmission >= transmissionStep * 1000)) {
    
    // 開始SPI傳輸
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    
    if (transmissionStep == 0) {
      // 第一次傳輸: 9顆LED - RGBRGBRGB (三種亮度: 0xFF/0x3F/0x0F)
      uint8_t data1[] = {
        // LED 1: Red (0xFF/0x00/0x00) - 最高亮度
        0xEE, 0xEE, 0xEE, 0xEE,  // Red: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 2: Green (0x00/0xFF/0x00) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Green: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 3: Blue (0x00/0x00/0xFF) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Blue: 0xFF -> EEEEEEEE
        // LED 4: Red (0x3F/0x00/0x00) - 中等亮度
        0x88, 0xE8, 0xEE, 0xEE,  // Red: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 5: Green (0x00/0x3F/0x00) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Green: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 6: Blue (0x00/0x00/0x3F) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Blue: 0x3F -> 88E8EEEE
        // LED 7: Red (0x0F/0x00/0x00) - 最低亮度
        0x88, 0x88, 0x8E, 0xEE,  // Red: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 8: Green (0x00/0x0F/0x00) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE,  // Green: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 9: Blue (0x00/0x00/0x0F) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE   // Blue: 0x0F -> 88888EEE
      };
      SPI.writeBytes(data1, 108);  // 9 LEDs × 12 bytes = 108 bytes
      
    } else if (transmissionStep == 1) {
      // 第二次傳輸: 9顆LED - GBRGBRGBR (三種亮度: 0xFF/0x3F/0x0F)
      uint8_t data2[] = {
        // LED 1: Green (0x00/0xFF/0x00) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Green: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 2: Blue (0x00/0x00/0xFF) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Blue: 0xFF -> EEEEEEEE
        // LED 3: Red (0xFF/0x00/0x00) - 最高亮度
        0xEE, 0xEE, 0xEE, 0xEE,  // Red: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 4: Green (0x00/0x3F/0x00) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Green: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 5: Blue (0x00/0x00/0x3F) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Blue: 0x3F -> 88E8EEEE
        // LED 6: Red (0x3F/0x00/0x00) - 中等亮度
        0x88, 0xE8, 0xEE, 0xEE,  // Red: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 7: Green (0x00/0x0F/0x00) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE,  // Green: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 8: Blue (0x00/0x00/0x0F) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE,  // Blue: 0x0F -> 88888EEE
        // LED 9: Red (0x0F/0x00/0x00) - 最低亮度
        0x88, 0x88, 0x8E, 0xEE,  // Red: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88   // Blue: 0x00 -> 88888888
      };
      SPI.writeBytes(data2, 108);  // 9 LEDs × 12 bytes = 108 bytes
      
    } else if (transmissionStep == 2) {
      // 第三次傳輸: 9顆LED - BRGBRGBRG (三種亮度: 0xFF/0x3F/0x0F)
      uint8_t data3[] = {
        // LED 1: Blue (0x00/0x00/0xFF) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Blue: 0xFF -> EEEEEEEE
        // LED 2: Red (0xFF/0x00/0x00) - 最高亮度
        0xEE, 0xEE, 0xEE, 0xEE,  // Red: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 3: Green (0x00/0xFF/0x00) - 最高亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0xEE, 0xEE, 0xEE, 0xEE,  // Green: 0xFF -> EEEEEEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 4: Blue (0x00/0x00/0x3F) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Blue: 0x3F -> 88E8EEEE
        // LED 5: Red (0x3F/0x00/0x00) - 中等亮度
        0x88, 0xE8, 0xEE, 0xEE,  // Red: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 6: Green (0x00/0x3F/0x00) - 中等亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0xE8, 0xEE, 0xEE,  // Green: 0x3F -> 88E8EEEE
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 7: Blue (0x00/0x00/0x0F) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE,  // Blue: 0x0F -> 88888EEE
        // LED 8: Red (0x0F/0x00/0x00) - 最低亮度
        0x88, 0x88, 0x8E, 0xEE,  // Red: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88,  // Green: 0x00 -> 88888888
        0x88, 0x88, 0x88, 0x88,  // Blue: 0x00 -> 88888888
        // LED 9: Green (0x00/0x0F/0x00) - 最低亮度
        0x88, 0x88, 0x88, 0x88,  // Red: 0x00 -> 88888888
        0x88, 0x88, 0x8E, 0xEE,  // Green: 0x0F -> 88888EEE
        0x88, 0x88, 0x88, 0x88   // Blue: 0x00 -> 88888888
      };
      SPI.writeBytes(data3, 108);  // 9 LEDs × 12 bytes = 108 bytes
    }
    
    // 結束SPI傳輸
    SPI.endTransaction();
    
    transmissionStep++;
    if (transmissionStep > 2) {
      transmissionStep = 3; // 保持在最後狀態直到5秒重置
    }
  }
}

/*
 * 額外測試函數 - 可以手動調用
 */
void sendCustomData(uint8_t data) {
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  SPI.transfer(data);
  SPI.endTransaction();
  
  Serial.print("發送自定義數據: 0x");
  Serial.print(data, HEX);
  Serial.print(" (");
  Serial.print(data, BIN);
  Serial.println(")");
}

/*
 * 連續發送多個相同數據
 */
void sendRepeatedData(uint8_t data, int count) {
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  
  for (int i = 0; i < count; i++) {
    SPI.transfer(data);
  }
  
  SPI.endTransaction();
  
  Serial.print("重複發送 ");
  Serial.print(count);
  Serial.print(" 次數據: 0x");
  Serial.println(data, HEX);
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
 * 發送8位數據並轉換為32位編碼 (新增功能)
 */
void send8bitAsEncoded(uint8_t data) {
  uint32_t encoded = convertTo32bit(data);
  send32bitData(encoded);
}
