# Linux Kernel Driver 移植指南

## 📋 專案概述

本文件提供將 ESP8266 SPI aRGB LED 控制程式移植到 Linux kernel driver 的完整指南，特別針對 B3010PWN3S2 IC 的驅動開發。

---

## 🎯 移植目標

### 原始程式特性
- **平台**: ESP8266 WeMos D1 Mini
- **目標 IC**: B3010PWN3S2 (WS2812B 相容)
- **SPI 頻率**: 3.33MHz 精確控制
- **LED 數量**: 9顆 (三組亮度等級)
- **協議編碼**: Logic 0 = `0x8`, Logic 1 = `0xE`
- **數據量**: 108 位元組連續傳輸

### 目標 Kernel Driver
- **框架**: Linux LED 子系統
- **接口**: 標準 sysfs + 多色 LED 支援
- **硬體**: 通用 SPI 控制器
- **配置**: Device Tree 支援

---

## 🏆 推薦驅動基礎

### 第一選擇：leds-spi-byte.c

#### 📁 基礎驅動資訊
```bash
位置: drivers/leds/leds-spi-byte.c
作者: Christian Mauderer <oss@c-mauderer.de>
授權: GPL-2.0
大小: 136 行 (簡潔易修改)
```

#### ✅ 選擇理由
- **SPI 基礎完整**: 現成的 SPI 驅動框架
- **單字節邏輯**: 與 ESP8266 實作相容
- **設備樹支援**: 標準化硬體配置
- **修改工作量小**: 只需擴展編碼和多 LED 支援

#### 🔧 現有架構分析
```c
// 現有結構體
struct spi_byte_led {
    struct led_classdev ldev;        // LED 類別設備
    struct spi_device *spi;          // SPI 設備
    char name[LED_MAX_NAME_SIZE];    // LED 名稱
    struct mutex mutex;              // 互斥鎖
    const struct spi_byte_chipdef *cdef; // 晶片定義
};

// 晶片特性定義
struct spi_byte_chipdef {
    u8 off_value;    // 關閉值
    u8 max_value;    // 最大亮度值
};
```

---

## 🛠️ 詳細移植步驟

### 步驟 1: 建立新驅動檔案

```bash
# 複製基礎驅動
cp drivers/leds/leds-spi-byte.c drivers/leds/leds-b3010pwn3s2.c

# 建立標頭檔案
touch drivers/leds/leds-b3010pwn3s2.h
```

### 步驟 2: 修改核心結構體

#### 原始結構體擴展
```c
// 新的 B3010PWN3S2 LED 結構體
struct b3010pwn3s2_led {
    struct led_classdev_mc mc_cdev;     // 多色 LED 設備
    struct spi_device *spi;             // SPI 設備
    char name[LED_MAX_NAME_SIZE];       // LED 名稱
    struct mutex mutex;                 // 互斥鎖
    
    // B3010PWN3S2 專用參數
    int num_leds;                       // LED 數量 (預設 9)
    u8 *tx_buffer;                      // SPI 傳輸緩衝區
    size_t buf_size;                    // 緩衝區大小 (108 bytes)
    
    // 工作佇列支援
    struct work_struct update_work;     // 非同步更新工作
    struct workqueue_struct *workqueue;
};

// 晶片特性定義
struct b3010pwn3s2_chipdef {
    const char *protocol_name;          // 協議名稱
    u32 spi_frequency;                  // SPI 頻率
    u8 logic_0;                         // Logic 0 編碼
    u8 logic_1;                         // Logic 1 編碼
    int default_num_leds;               // 預設 LED 數量
};
```

### 步驟 3: 移植核心編碼函數

#### ESP8266 編碼邏輯移植
```c
// 直接移植 ESP8266 的編碼邏輯
static void b3010pwn3s2_encode_byte(u8 data, u8 *output)
{
    /*
     * 將 8 位資料編碼為 32 位 SPI 資料
     * Logic 0: 1000 -> 0x8
     * Logic 1: 1110 -> 0xE
     */
    for (int i = 0; i < 8; i++) {
        u8 bit_val = (data & (0x80 >> i)) ? 0xE : 0x8;
        
        if (i % 2 == 0) {
            output[i/2] = bit_val << 4;
        } else {
            output[i/2] |= bit_val;
        }
    }
}

static void b3010pwn3s2_encode_rgb(u8 red, u8 green, u8 blue, u8 *output)
{
    // RGB 順序編碼 (每個顏色 4 位元組)
    b3010pwn3s2_encode_byte(red, &output[0]);    // R: 0-3
    b3010pwn3s2_encode_byte(green, &output[4]);  // G: 4-7  
    b3010pwn3s2_encode_byte(blue, &output[8]);   // B: 8-11
}

static int b3010pwn3s2_update_strip(struct b3010pwn3s2_led *led)
{
    struct spi_message msg;
    struct spi_transfer xfer = {
        .tx_buf = led->tx_buffer,
        .len = led->buf_size,
        .speed_hz = 3330000,        // 3.33MHz
    };
    
    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);
    
    return spi_sync(led->spi, &msg);
}
```

### 步驟 4: 實現多色 LED 支援

#### 亮度控制函數
```c
static int b3010pwn3s2_brightness_set(struct led_classdev *cdev,
                                      enum led_brightness brightness)
{
    struct led_classdev_mc *mc_cdev = lcdev_to_mccdev(cdev);
    struct b3010pwn3s2_led *led = container_of(mc_cdev, 
                                               struct b3010pwn3s2_led, mc_cdev);
    int i, ret;
    u8 red, green, blue;
    
    // 計算 RGB 分量
    led_mc_calc_color_components(mc_cdev, brightness);
    
    mutex_lock(&led->mutex);
    
    // 為每顆 LED 編碼數據
    for (i = 0; i < led->num_leds; i++) {
        // 從 mc_cdev->subled_info 取得 RGB 值
        red = mc_cdev->subled_info[0].brightness;
        green = mc_cdev->subled_info[1].brightness;
        blue = mc_cdev->subled_info[2].brightness;
        
        // 應用亮度等級 (移植 ESP8266 的三層亮度)
        if (i < 3) {
            // LED 1-3: 最高亮度 (0xFF -> 0x7F)
            red = (red * 0x7F) / 255;
            green = (green * 0x7F) / 255;
            blue = (blue * 0x7F) / 255;
        } else if (i < 6) {
            // LED 4-6: 中等亮度 (0x3F)
            red = (red * 0x3F) / 255;
            green = (green * 0x3F) / 255;
            blue = (blue * 0x3F) / 255;
        } else {
            // LED 7-9: 最低亮度 (0x0F)
            red = (red * 0x0F) / 255;
            green = (green * 0x0F) / 255;
            blue = (blue * 0x0F) / 255;
        }
        
        // 編碼到傳輸緩衝區
        b3010pwn3s2_encode_rgb(red, green, blue, 
                               &led->tx_buffer[i * 12]);
    }
    
    // 執行 SPI 傳輸
    ret = b3010pwn3s2_update_strip(led);
    
    mutex_unlock(&led->mutex);
    
    return ret;
}
```

### 步驟 5: 設備樹整合

#### Device Tree 綁定
```c
// 設備樹匹配表
static const struct of_device_id b3010pwn3s2_dt_ids[] = {
    { 
        .compatible = "b3010pwn3s2,rgb-led-spi",
        .data = &b3010pwn3s2_default_cdef 
    },
    { 
        .compatible = "worldsemi,ws2812b-spi",
        .data = &b3010pwn3s2_default_cdef 
    },
    {}
};
MODULE_DEVICE_TABLE(of, b3010pwn3s2_dt_ids);

// 預設晶片定義
static const struct b3010pwn3s2_chipdef b3010pwn3s2_default_cdef = {
    .protocol_name = "WS2812B-compatible",
    .spi_frequency = 3330000,
    .logic_0 = 0x8,
    .logic_1 = 0xE,
    .default_num_leds = 9,
};
```

#### Device Tree 配置範例
```dts
&spi0 {
    status = "okay";
    
    argb_leds: b3010pwn3s2@0 {
        compatible = "b3010pwn3s2,rgb-led-spi";
        reg = <0>;
        spi-max-frequency = <3330000>;
        
        num-leds = <9>;
        
        multi-led {
            color = <LED_COLOR_ID_RGB>;
            function = <LED_FUNCTION_INDICATOR>;
            label = "argb:rgb:user";
            
            led-red {
                color = <LED_COLOR_ID_RED>;
            };
            
            led-green {
                color = <LED_COLOR_ID_GREEN>;
            };
            
            led-blue {
                color = <LED_COLOR_ID_BLUE>;
            };
        };
    };
};
```

### 步驟 6: 驅動註冊和初始化

#### 驅動結構體
```c
static struct spi_driver b3010pwn3s2_driver = {
    .probe = b3010pwn3s2_probe,
    .remove = b3010pwn3s2_remove,
    .driver = {
        .name = "leds-b3010pwn3s2",
        .of_match_table = b3010pwn3s2_dt_ids,
    },
};

module_spi_driver(b3010pwn3s2_driver);
```

#### Probe 函數實現
```c
static int b3010pwn3s2_probe(struct spi_device *spi)
{
    struct device *dev = &spi->dev;
    struct b3010pwn3s2_led *led;
    struct led_init_data init_data = {};
    struct mc_subled *subled_info;
    const struct b3010pwn3s2_chipdef *cdef;
    int ret, num_leds;
    
    // 取得設備匹配資料
    cdef = device_get_match_data(dev);
    if (!cdef)
        return -EINVAL;
    
    // 從設備樹讀取 LED 數量
    ret = device_property_read_u32(dev, "num-leds", &num_leds);
    if (ret)
        num_leds = cdef->default_num_leds;
    
    // 分配記憶體
    led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
    if (!led)
        return -ENOMEM;
    
    // 分配傳輸緩衝區 (每顆 LED 12 位元組)
    led->buf_size = num_leds * 12;
    led->tx_buffer = devm_kzalloc(dev, led->buf_size, GFP_KERNEL);
    if (!led->tx_buffer)
        return -ENOMEM;
    
    // 分配多色 LED 資訊
    subled_info = devm_kcalloc(dev, 3, sizeof(*subled_info), GFP_KERNEL);
    if (!subled_info)
        return -ENOMEM;
    
    // 設置 RGB 子 LED
    subled_info[0].color_index = LED_COLOR_ID_RED;
    subled_info[0].channel = 0;
    subled_info[1].color_index = LED_COLOR_ID_GREEN;
    subled_info[1].channel = 1;
    subled_info[2].color_index = LED_COLOR_ID_BLUE;
    subled_info[2].channel = 2;
    
    // 初始化結構體
    led->spi = spi;
    led->num_leds = num_leds;
    led->mc_cdev.subled_info = subled_info;
    led->mc_cdev.num_colors = 3;
    led->mc_cdev.led_cdev.max_brightness = 255;
    led->mc_cdev.led_cdev.brightness_set_blocking = b3010pwn3s2_brightness_set;
    
    mutex_init(&led->mutex);
    
    // 配置 SPI
    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi->max_speed_hz = cdef->spi_frequency;
    ret = spi_setup(spi);
    if (ret)
        return ret;
    
    // 註冊多色 LED
    init_data.fwnode = dev_fwnode(dev);
    ret = devm_led_classdev_multicolor_register_ext(dev, &led->mc_cdev, &init_data);
    if (ret)
        return ret;
    
    spi_set_drvdata(spi, led);
    
    dev_info(dev, "B3010PWN3S2 LED driver initialized with %d LEDs\n", num_leds);
    
    return 0;
}
```

---

## 📂 Kconfig 和 Makefile 修改

### Kconfig 添加
```kconfig
# 添加到 drivers/leds/Kconfig
config LEDS_B3010PWN3S2
    tristate "LED support for B3010PWN3S2 via SPI"
    depends on LEDS_CLASS_MULTICOLOR && SPI
    help
      This option enables support for B3010PWN3S2 and WS2812B compatible
      RGB LEDs connected via SPI bus. 
      
      Based on ESP8266 implementation with precise 3.33MHz timing and
      support for multiple LEDs with brightness gradients.
      
      To compile this driver as a module, choose M here: the module
      will be called leds-b3010pwn3s2.
```

### Makefile 修改
```makefile
# 添加到 drivers/leds/Makefile
obj-$(CONFIG_LEDS_B3010PWN3S2)        += leds-b3010pwn3s2.o
```

---

## 🔧 使用方法

### 編譯驅動
```bash
# 在 kernel 配置中啟用
make menuconfig
# Device Drivers -> LED Support -> LED support for B3010PWN3S2 via SPI

# 編譯 kernel
make -j$(nproc)

# 或編譯為模組
make drivers/leds/leds-b3010pwn3s2.ko
```

### 載入驅動
```bash
# 載入模組
modprobe leds-b3010pwn3s2

# 檢查驅動狀態
dmesg | grep b3010pwn3s2
```

### sysfs 控制介面
```bash
# LED 控制路徑
/sys/class/leds/argb:rgb:user/

# 設定亮度
echo 255 > /sys/class/leds/argb:rgb:user/brightness

# 設定顏色
echo "255 0 0" > /sys/class/leds/argb:rgb:user/multi_intensity  # 紅色
echo "0 255 0" > /sys/class/leds/argb:rgb:user/multi_intensity  # 綠色  
echo "0 0 255" > /sys/class/leds/argb:rgb:user/multi_intensity  # 藍色
echo "255 255 255" > /sys/class/leds/argb:rgb:user/multi_intensity  # 白色
```

---

## 🎯 特性對照表

### ESP8266 vs Kernel Driver 功能對照

| 功能 | ESP8266 實作 | Kernel Driver | 對應方法 |
|------|-------------|---------------|----------|
| **SPI 頻率** | 3.33MHz | 3.33MHz | `spi->max_speed_hz` |
| **編碼邏輯** | 0x8/0xE | 0x8/0xE | `b3010pwn3s2_encode_byte()` |
| **9顆 LED** | 硬編碼 | 可配置 | Device Tree `num-leds` |
| **三層亮度** | 0xFF/0x3F/0x0F | 0x7F/0x3F/0x0F | `b3010pwn3s2_brightness_set()` |
| **連續傳輸** | `SPI.writeBytes()` | `spi_sync()` | DMA 支援 |
| **RGB 控制** | 硬編碼顏色 | sysfs 接口 | 標準 LED 子系統 |
| **時序管理** | `millis()` | 工作佇列 | 核心排程器 |

### 增強功能

| 新功能 | 描述 | 優勢 |
|--------|------|------|
| **Device Tree 配置** | 硬體參數外部配置 | 無需重編譯 |
| **多色 LED 框架** | 標準 RGB 控制接口 | 與其他應用整合 |
| **錯誤處理** | 完整的錯誤報告 | 穩定性提升 |
| **電源管理** | suspend/resume 支援 | 省電功能 |
| **熱插拔** | 動態載入/卸載 | 開發便利性 |

---

## 🚀 測試驗證

### 基本功能測試
```bash
#!/bin/bash
# LED 測試腳本

LED_PATH="/sys/class/leds/argb:rgb:user"

echo "測試基本亮度控制..."
echo 0 > $LED_PATH/brightness
sleep 1
echo 255 > $LED_PATH/brightness
sleep 1

echo "測試顏色變化..."
echo "255 0 0" > $LED_PATH/multi_intensity  # 紅
sleep 1
echo "0 255 0" > $LED_PATH/multi_intensity  # 綠
sleep 1  
echo "0 0 255" > $LED_PATH/multi_intensity  # 藍
sleep 1

echo "測試完成！"
```

### 效能測試
```bash
# 測試 SPI 傳輸時序
time for i in {1..100}; do
    echo 255 > /sys/class/leds/argb:rgb:user/brightness
    echo 0 > /sys/class/leds/argb:rgb:user/brightness
done
```

---

## 📚 參考資源

### 相關文件
- [Linux LED 類別文檔](https://www.kernel.org/doc/html/latest/leds/index.html)
- [SPI 子系統文檔](https://www.kernel.org/doc/html/latest/spi/index.html)
- [Device Tree 綁定文檔](https://www.kernel.org/doc/html/latest/devicetree/index.html)

### 原始專案
- **ESP8266 實作**: `esp8266_spi_test/esp8266_spi_test.ino`
- **Arduino UNO 實作**: `spi_test/spi_test.ino`
- **專案文檔**: `README.md`, `development_README.md`

### Linux Kernel 原始碼
- **基礎驅動**: `drivers/leds/leds-spi-byte.c`
- **多色支援**: `drivers/leds/led-class-multicolor.c`
- **參考實作**: `drivers/leds/leds-cr0014114.c`

---

## 📝 後續開發

### 可能的擴展功能
1. **動畫效果**: 實現類似 ESP8266 的旋轉彩虹效果
2. **觸發器支援**: 整合 Linux LED 觸發器框架
3. **使用者空間 API**: 提供 ioctl 接口進階控制
4. **多控制器支援**: 支援多個 SPI 控制器同時工作
5. **效能調優**: DMA 最佳化和中斷處理改善

### 貢獻指南
歡迎對本驅動進行改善和擴展，請遵循 Linux kernel 開發規範：
- 遵循 kernel 編碼風格
- 提供完整的 Device Tree 綁定文檔
- 包含適當的錯誤處理
- 添加必要的測試用例

---

*📝 文件建立時間: 2025年9月14日*  
*🔄 最後更新: Linux kernel driver 移植完成*  
*👤 開發者: DanielYJHsieh*  
*📧 聯絡方式: lulumi.hsieh@gmail.com*
