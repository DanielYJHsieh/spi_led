# SPI 控制 aRGB LED 專案

本專案基於 [Everstars 單線式RGB+IC應用手冊](https://www.everstars.com.cn/news_details_263014.html) 中的虛擬碼實現，提供 Arduino UNO 和 ESP8266 雙平台的 SPI aRGB LED 控制解決方案。

## 專案特色

- 🎯 基於 Everstars 官方虛擬碼實現
- 🔧 雙平台支援：Arduino UNO 和 ESP8266 WeMos D1 Mini
- 🌈 支援多顆 aRGB LED 串聯控制
- ⚡ 最佳化 SPI 時鐘頻率，確保穩定通訊
- 🎨 內建顏色旋轉動畫效果
- 📊 完整的邏輯分析儀測試驗證

## 實作版本

### 📁 `spi_test/` - Arduino UNO 實作
- **適用平台**: Arduino UNO (ATmega328P)
- **SPI 頻率**: 4MHz (受限於系統時鐘)
- **特色**: 模擬 writeBytes 功能，作為概念驗證

### 📁 `esp8266_spi_test/` - ESP8266 進階實作
- **適用平台**: ESP8266 WeMos D1 Mini
- **SPI 頻率**: 3.33MHz (精確頻率控制)
- **特色**: 硬體 writeBytes 支援，18顆 LED 控制，UART命令介面

## 硬體需求

### Arduino UNO 版本 (`spi_test/`)
- Arduino UNO 開發板 × 1
- aRGB LED (如 WS2812B、WS2812、SK6812 等) × 3 (可調整)

### ESP8266 版本 (`esp8266_spi_test/`)  
- ESP8266 WeMos D1 Mini × 1
- aRGB LED × 9 (測試用)
- 邏輯分析儀 (建議)

### 通用組件
- 跳線若干
- 麵包板 (可選)

### 引腳連接

#### Arduino UNO (`spi_test/`)

| Arduino UNO | aRGB LED | 說明 |
|-------------|----------|------|
| Pin 11 (MOSI) | DIN | 數據輸入線 |
| 5V | VCC | 電源正極 |
| GND | GND | 電源負極 |
| Pin 13 (SCK) | - | 未使用 (單線協議) |

#### ESP8266 WeMos D1 Mini (`esp8266_spi_test/`)

| ESP8266 | aRGB LED | 說明 |
|---------|----------|------|
| D7 (GPIO13) | DIN | 數據輸入線 |
| 5V | VCC | 電源正極 |
| GND | GND | 電源負極 |
| 任意 USB 轉 TTL | RX/TX | UART命令控制 (115200 bps) |

```
Arduino UNO                    aRGB LED Strip
┌─────────────┐               ┌─────┐ ┌─────┐ ┌─────┐
│             │               │LED1 │ │LED2 │ │LED3 │
│   Pin 11────┼───────────────┤DIN  │ │     │ │     │
│   (MOSI)    │               │     │ │     │ │     │
│             │               │DOUT─┤ │DIN  │ │     │
│    5V───────┼───────────────┤VCC  │ │     │ │     │
│             │               │     │ │DOUT─┤ │DIN  │
│   GND───────┼───────────────┤GND  │ │     │ │     │
│             │               └─────┘ └─────┘ └─────┘
└─────────────┘

ESP8266 WeMos D1 Mini          aRGB LED Strip (18 LEDs)
┌─────────────┐               ┌─────┐ ┌─────┐     ┌─────┐
│             │               │LED1 │ │LED2 │ ... │LED18│
│   D7────────┼───────────────┤DIN  │ │     │     │     │
│   (GPIO13)  │               │     │ │     │     │     │
│             │               │DOUT─┤ │DIN  │ ... │     │
│    5V───────┼───────────────┤VCC  │ │     │     │     │
│             │               │     │ │     │ ... │     │
│   GND───────┼───────────────┤GND  │ │     │     │     │
│             │               └─────┘ └─────┘     └─────┘
│   USB-SERIAL│
└─────────────┘
```

## 協議說明

本專案實現的通訊協議遵循 aRGB LED 標準：

### 時序規格
- **Arduino UNO**: 4MHz SPI (最接近 3.33MHz 的可達頻率)
- **ESP8266**: 3.33MHz SPI (精確頻率控制)
- **Logic 0**: 300ns High + 900ns Low (對應 SPI 的 `1000`)
- **Logic 1**: 900ns High + 300ns Low (對應 SPI 的 `1110`)
- **Latch Time**: >50µs 低電平

### 數據格式
每個 LED 需要 24 位數據：
- R 分量：8 位 (MSB 先送)
- G 分量：8 位
- B 分量：8 位

多個 LED 串聯時的發送順序：
```
LED1(R→G→B) → LED2(R→G→B) → LED3(R→G→B) → Latch Time
```

## 軟體說明

### 核心功能

1. **數據轉換** (`convertColorTo32bit`)
   - 將 8 位顏色值轉換為 32 位 SPI 數據
   - 實現 Logic 0/1 的編碼規則

2. **SPI 傳輸** (`send32bitData`)
   - 通過 SPI 發送 32 位編碼數據
   - 自動處理字節順序

3. **LED 控制** (`sendLEDData`)
   - 按序發送所有 LED 的 RGB 數據
   - 自動添加 Latch Time

4. **動畫效果** (`rotateLEDColors`)
   - 顏色旋轉動畫
   - 可擴展其他動畫模式

### 主要函數

```cpp
// 設定單個 LED 顏色
void setLEDColor(int ledIndex, uint8_t red, uint8_t green, uint8_t blue);

// 設定所有 LED 相同顏色
void setAllLEDs(uint8_t red, uint8_t green, uint8_t blue);

// 清除所有 LED
void clearAllLEDs();

// 發送 LED 數據
void sendLEDData();
```

## 使用方法

### 選擇適合的版本
- **Arduino UNO**: 適合學習和原型開發
- **ESP8266**: 適合需要精確時序的應用

### 1. 硬體連接
按照上述引腳連接表連接電路。

### 2. 編譯上傳

#### Arduino UNO 版本
1. 打開 Arduino IDE
2. 載入 `spi_test/spi_test.ino`
3. 選擇開發板：Arduino UNO
4. 編譯並上傳程式

#### ESP8266 版本  
1. 打開 Arduino IDE
2. 安裝 ESP8266 開發板套件
3. 載入 `esp8266_spi_test/esp8266_spi_test.ino`
4. 選擇開發板：WeMos D1 Mini
5. 編譯並上傳程式

### 3. 觀察效果

#### Arduino UNO 版本
- 程式啟動後會顯示：LED1=紅色、LED2=綠色、LED3=藍色
- 每 2 秒顏色會向右旋轉一個位置
- 串列監視器會顯示詳細的執行資訊

#### ESP8266 版本
- 支援18顆 LED，預設全白燈
- 通過UART命令控制每個LED的顏色
- 提供多種控制命令：設置、狀態、重置等
- 適合實際WS2812B LED燈條控制應用

### 4. 自訂設定

#### Arduino UNO 版本
修改 LED 數量：
```cpp
#define NUM_LEDS 5  // 改為 5 顆 LED
```

修改初始顏色：
```cpp
void setup() {
  // ...
  setLEDColor(0, 255, 0, 255);  // 設定 LED1 為紫色
  setLEDColor(1, 255, 255, 0);  // 設定 LED2 為黃色
  // ...
}
```

#### ESP8266 版本
修改 LED 數量：
```cpp
// 在程式開頭修改常數
const uint8_t NUM_LEDS = 30;  // 改為 30 顆 LED
```

控制LED顏色：
```
# 透過Serial Monitor發送以下命令
0,255,0,0     # 將第1顆LED設為紅色
1,0,255,0     # 將第2顆LED設為綠色
reset         # 所有LED重設為白色
status        # 檢視所有LED狀態
```
```cpp
#define NUM_LEDS 12  // 改為 12 顆 LED
```

調整亮度等級：
```cpp
uint8_t levels[] = {0xFF, 0x7F, 0x1F};  // 自訂亮度
```

## 性能比較

| 特性 | Arduino UNO | ESP8266 |
|------|-------------|---------|
| SPI 頻率 | 4MHz (非精確) | 3.33MHz (精確) |
| writeBytes 支援 | 模擬實現 | 硬體支援 |
| 時鐘穩定性 | 良好 | 優秀 |
| LED 數量 | 3顆 (測試) | 9顆 (完整) |
| 適用場景 | 學習原型 | 產品應用 |

## 測試工具

### 邏輯分析儀設定
- **取樣率**: 建議 100MHz 以上
- **觸發**: MOSI 上升沿
- **協議**: SPI 模式解碼
- **觀察重點**: 
  - Arduino: 12 位元組連續傳輸
  - ESP8266: 108 位元組連續傳輸

## 故障排除

### 平台選擇建議
- **學習和實驗**: 選擇 Arduino UNO 版本
- **精確時序要求**: 選擇 ESP8266 版本
- **大量 LED 控制**: 建議使用 ESP8266 版本

### 常見問題

1. **LED 不亮**
   - 檢查電源連接 (5V, GND)
   - 確認數據線連接到 Pin 11
   - 檢查 LED 是否支援 5V 供電

2. **顏色不正確**
   - 確認 LED 型號的 RGB 順序 (可能是 GRB)
   - 檢查 SPI 設定是否正確

3. **時序不穩定 (ESP8266 專用)**
   - 檢查電源供應是否穩定
   - 避免 WiFi 干擾，可考慮關閉 WiFi
   - 使用邏輯分析儀驗證 SPI 時序

4. **Arduino UNO 頻率問題**
   - 接受 4MHz 而非 3.33MHz 的限制
   - 考慮升級到 ESP8266 獲得更好的頻率控制

### 電路改善建議

```
Arduino Pin 11 ──[33Ω]── aRGB LED DIN
                           │
5V ──[100µF]──[0.1µF]── VCC
│                        │
└── GND ──────────────── GND
```

## 擴展功能

### 1. 添加更多動畫效果
```cpp
void rainbowEffect() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    setLEDColor(i, hueToRGB(hue + i * 85)); // 彩虹效果
  }
  hue += 5;
}
```

### 2. 串列命令控制
```cpp
void processSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readString();
    // 解析命令並設定顏色
  }
}
```

### 3. 感測器控制
```cpp
void readSensor() {
  int sensorValue = analogRead(A0);
  uint8_t brightness = map(sensorValue, 0, 1023, 0, 255);
  setAllLEDs(brightness, 0, 0); // 根據感測器調整亮度
}
```

## 專案文件

### 📚 技術文件
- `aRGB_LED_Data_Structure.md` - aRGB LED 資料結構詳解
- `LA_Testing_Guide.md` - 邏輯分析儀測試指南  
- `PowerPoint_Content_Guide.md` - PowerPoint 簡報製作指南
- `development_README.md` - 完整開發歷程記錄
- `Linux_Kernel_Driver_Porting_Guide.md` - Linux kernel driver 移植指南

### ⚙️ 配置檔案
- `project_config.json` - 專案配置設定
- `advanced_examples.ino` - 進階功能範例
- `wiring_diagram.txt` - 詳細接線說明

## 技術參考

- [Everstars aRGB 應用手冊](https://www.everstars.com.cn/news_details_263014.html)
- [Arduino SPI 庫文檔](https://www.arduino.cc/en/reference/SPI)
- [WS2812B 規格書](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)

## 授權

本專案基於 MIT 授權條款開源。

## 貢獻

歡迎提交 Issue 和 Pull Request 來改善本專案！
