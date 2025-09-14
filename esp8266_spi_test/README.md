# ESP8266 SPI aRGB LED 測試程式

## 專案概述
這是一個基於ESP8266 WeMos D1 Mini的SPI aRGB LED控制測試程式，用於驗證SPI傳輸穩定性和aRGB LED協議實現。

## 硬體需求
- **主控板**: ESP8266 WeMos D1 Mini
- **測試工具**: 邏輯分析儀 (Logic Analyzer)
- **目標設備**: WS2812B aRGB LED燈條 (可選)

## 硬體連接
```
ESP8266 WeMos D1 Mini → 邏輯分析儀
├── D7 (GPIO13) MOSI → CH0
├── D5 (GPIO14) SCK  → CH1
└── GND             → GND
```

## SPI配置
- **頻率**: 3.33MHz (精確支援)
- **模式**: SPI_MODE0 (CPOL=0, CPHA=0)
- **位元順序**: MSBFIRST
- **數據長度**: 108位元組 (9顆LED × 12位元組/LED)

## aRGB LED協議編碼
程式實現WS2812B單線協議的SPI模擬：
- **Logic 0**: `1000` (300ns High + 900ns Low) → 0x8
- **Logic 1**: `1110` (900ns High + 300ns Low) → 0xE

### 亮度等級
- **LED 1-3**: 0xFF (最高亮度)
- **LED 4-6**: 0x3F (中等亮度) 
- **LED 7-9**: 0x0F (最低亮度)

## 功能特色
1. **9顆LED控制**: 模擬真實LED燈條
2. **三種亮度漸變**: 測試不同編碼模式
3. **旋轉彩虹效果**: RGB顏色輪播
4. **連續傳輸**: 使用`SPI.writeBytes()`確保時鐘穩定性
5. **定時輪播**: 每秒切換一種模式，5秒重新循環

## 輪播模式
### Data1 - RGBRGBRGB (T=0秒)
- LED 1,4,7: Red
- LED 2,5,8: Green  
- LED 3,6,9: Blue

### Data2 - GBRGBRGBR (T=1秒)
- LED 1,4,7: Green
- LED 2,5,8: Blue
- LED 3,6,9: Red

### Data3 - BRGBRGBRG (T=2秒)
- LED 1,4,7: Blue
- LED 2,5,8: Red
- LED 3,6,9: Green

## 邏輯分析儀設定
- **取樣率**: 建議100MHz以上
- **觸發**: MOSI上升沿
- **協議**: SPI模式解碼
- **觀察**: 108位元組連續傳輸無間隔

## 技術優勢
- **精確頻率**: ESP8266支援3.33MHz精確設定
- **硬體writeBytes**: 單次DMA傳輸，時鐘連續性最佳
- **豐富測試**: 三種亮度×三種顏色×三種排列 = 完整測試矩陣
- **實用性**: 可直接連接WS2812B LED進行實際驅動

## 使用說明
1. 將程式上傳到ESP8266
2. 連接邏輯分析儀到指定腳位
3. 觀察SPI傳輸波形和協議解碼
4. 驗證108位元組數據的完整性和時序

## 注意事項
- 程式中已註解掉Serial輸出避免干擾SPI時鐘
- 建議使用電源供應器而非USB供電以確保訊號穩定
- 如連接實際LED，注意電流消耗和電壓準位匹配
