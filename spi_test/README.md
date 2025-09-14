# Arduino UNO SPI aRGB LED 測試程式

## 專案概述
這是一個基於Arduino UNO的SPI aRGB LED控制測試程式，作為ESP8266版本的前導開發和比較基準。

## 硬體需求
- **主控板**: Arduino UNO (ATmega328P)
- **測試工具**: 邏輯分析儀 (Logic Analyzer)
- **目標設備**: WS2812B aRGB LED燈條 (可選)

## 硬體連接
```
Arduino UNO → 邏輯分析儀
├── Pin 11 (MOSI) → CH0
├── Pin 13 (SCK)  → CH1
└── GND           → GND
```

## SPI配置
- **頻率**: 4MHz (最接近3.33MHz的可達頻率)
- **模式**: SPI_MODE0 (CPOL=0, CPHA=0)
- **位元順序**: MSBFIRST
- **數據長度**: 12位元組 (3組RGB數據)

## 硬體限制
### 頻率限制
Arduino UNO的SPI頻率選項受16MHz系統時鐘限制：
- 16MHz/2 = 8MHz
- 16MHz/4 = 4MHz ← **實際使用**
- 16MHz/8 = 2MHz
- 16MHz/16 = 1MHz

無法精確達到aRGB LED的理想3.33MHz頻率。

### SPI庫限制
Arduino標準SPI庫不支援`writeBytes()`，需要使用：
- `spi_writeBytes()` 模擬函數
- 迴圈調用`SPI.transfer()`實現連續傳輸

## aRGB LED協議編碼
實現WS2812B單線協議的SPI模擬：
- **Logic 0**: `1000` → 0x8
- **Logic 1**: `1110` → 0xE

### 測試數據
程式包含三組測試數據：
- **0xAA** (10101010) → `0xE8, 0xE8, 0xE8, 0xE8`
- **0x00** (00000000) → `0x88, 0x88, 0x88, 0x88`
- **0xFF** (11111111) → `0xEE, 0xEE, 0xEE, 0xEE`

## 功能模組

### 核心函數
1. **`spi_writeBytes()`**: 模擬ESP8266的writeBytes功能
2. **`convertTo32bit()`**: 8位轉32位aRGB編碼
3. **`send32bitData()`**: 發送32位編碼數據
4. **`sendContinuousTestData()`**: 連續測試數據傳輸

### 編碼轉換
```c
uint32_t convertTo32bit(uint8_t data) {
  // 將每個位元轉換為4位aRGB編碼
  // 0 → 1000 (0x8)
  // 1 → 1110 (0xE)
}
```

## 測試模式
程式支援三種編譯選項：
1. **原始8位測試**: 直接傳輸0x55, 0x00, 0xFF
2. **連續傳輸測試**: 使用模擬writeBytes功能
3. **函數調用測試**: 使用編碼轉換函數

## 時鐘穩定性
為確保SPI時鐘穩定：
- 註解掉所有Serial輸出
- 在beginTransaction和endTransaction間連續傳輸
- 避免中斷干擾

## 性能特性
- **單次傳輸**: 12位元組連續傳輸
- **時鐘連續性**: 在Arduino限制下達到最佳效果
- **代碼重用性**: 函數化設計便於移植到其他平台
- **測試完整性**: 涵蓋不同位元模式的編碼測試

## 使用說明
1. 選擇適當的編譯選項(#if/#elif/#else)
2. 上傳程式到Arduino UNO
3. 連接邏輯分析儀
4. 觀察SPI波形和協議解碼
5. 與ESP8266版本比較時序精度

## 限制與改進
### 已知限制
- SPI頻率無法精確達到3.33MHz
- 無硬體writeBytes支援
- 系統時鐘較低影響時序精度

### 改進方向
- 移植到ESP8266獲得更好的頻率控制
- 使用DMA傳輸提升效率
- 增加更多測試模式驗證穩定性

## 技術筆記
此版本主要作為概念驗證和學習平台，為ESP8266的進階實現提供基礎。雖然受硬體限制，但成功驗證了SPI模擬aRGB協議的可行性。
