/*
 * Arduino UNO SPI數據傳輸測試
 * 純SPI測試，不使用CS信號
 * 
 * 硬體連接:
 * - MOSI (Pin 11) -> 邏輯分析儀 CH0
 * - SCK (Pin 13) -> 邏輯分析儀 CH1  
 * - GND -> 邏輯分析儀 GND
 * 
 * LA SPI設定:
 * - MOSI: CH0, SCK: CH1
 * - 時鐘極性: CPOL=0, 時鐘相位: CPHA=0
 */

#include <SPI.h>

// Pin 定義
#define MOSI_PIN 11  // SPI MOSI
#define SCK_PIN 13   // SPI Clock

// SPI 設定參數
// Arduino UNO SPI頻率選項 (16MHz系統時鐘分頻):
// 16MHz/2 = 8MHz
// 16MHz/4 = 4MHz  
// 16MHz/8 = 2MHz  ← 目前實際頻率
// 16MHz/16 = 1MHz
// 最接近3.33MHz的是4MHz (16MHz/4)
const uint32_t SPI_FREQUENCY = 4000000;  // 4MHz (最接近3.33MHz的實際可達頻率)

void setup() {
  // === 串列通訊可能影響SPI時鐘穩定性 ===
  // Serial.begin(115200);  // 串列初始化可能影響時鐘
  // Serial.println("Arduino UNO SPI測試程式 (無CS信號)");
  // Serial.println("測試SPI基本傳輸功能");
  
  // 禁用中斷以獲得最穩定的SPI時鐘
  // noInterrupts();  // 可選：完全禁用中斷
  
  // 初始化SPI
  SPI.begin();
  
  // === 註解掉可能造成時鐘干擾的串列輸出 ===
  // Serial.println("SPI初始化完成");
  // Serial.println("MOSI: Pin 11, SCK: Pin 13");
  // Serial.println("SPI頻率: 4MHz (最接近3.33MHz的實際可達頻率)");
  // Serial.println("LA設定: CH0=MOSI, CH1=SCK");
  // Serial.println("注意: 標準SPI中MOSI空閒時為高電平");
  // Serial.println();
}

void loop() {
  static bool testSent = false;
  
  if (!testSent) {
    // === 可能造成CLK不穩定的原因 ===
    // Serial.println("=== 開始SPI測試傳輸 ===");  // 串列輸出會中斷SPI時鐘
    
    // 開始SPI傳輸 - 一次性傳輸所有數據，避免間隔
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    
    // 連續發送所有測試數據，無間隔
#if 0
    // 原始8位數據測試
    SPI.transfer(0x55);  // 測試數據2: 01010101
    SPI.transfer(0x00);  // 測試數據3: 00000000
    SPI.transfer(0xFF);  // 測試數據4: 11111111
#elif 1
    // 使用模擬writeBytes進行連續傳送
    sendContinuousTestData();
#else
    // 8位轉32位編碼測試 (aRGB LED協議)
    send8bitAsEncoded(0xAA);  // 10101010 -> E8E8E8E8
    send8bitAsEncoded(0x00);  // 00000000 -> 88888888
    send8bitAsEncoded(0xFF);  // 11111111 -> EEEEEEEE
#endif    
    // 結束SPI傳輸
    SPI.endTransaction();
    
    // Serial.println("=== SPI測試傳輸完成 ===");  // 串列輸出會中斷SPI時鐘
    // Serial.println("程式進入idle狀態");  // 串列輸出會中斷SPI時鐘
    
    //testSent = true;  // 重新啟用，只發送一次避免重複中斷
  }
  
  // 維持idle狀態
  delay(5000);  // 長時間delay可能影響系統穩定性，改用短delay
  //delay(100);
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
 * 發送32位數據 (分解為4個8位)
 */
void send32bitData(uint32_t data) {
  // 從MSB到LSB逐字節發送
  for (int i = 3; i >= 0; i--) {
    uint8_t byte_data = (data >> (i * 8)) & 0xFF;
    SPI.transfer(byte_data);
  }
}

/*
 * 發送8位數據並轉換為32位編碼 (新增功能)
 */
void send8bitAsEncoded(uint8_t data) {
  uint32_t encoded = convertTo32bit(data);
  send32bitData(encoded);
}

/*
 * Arduino UNO版本的writeBytes模擬函數
 * 連續傳送多個位元組，模擬ESP8266的SPI.writeBytes()
 */
void spi_writeBytes(uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    SPI.transfer(data[i]);
  }
}

/*
 * 連續傳送測試數據 (使用模擬writeBytes)
 */
void sendContinuousTestData() {
  uint8_t test_data[] = {
    // 0xAA (10101010) -> E8E8E8E8
    0xE8, 0xE8, 0xE8, 0xE8,
    // 0x00 (00000000) -> 88888888
    0x88, 0x88, 0x88, 0x88,
    // 0xFF (11111111) -> EEEEEEEE
    0xEE, 0xEE, 0xEE, 0xEE
  };
  
  // 使用模擬的writeBytes函數
  spi_writeBytes(test_data, 12);
}
