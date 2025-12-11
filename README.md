# uart_tof_g431_stm32_test
README.md（可直接使用）
# STM32 TOF(UART-DMA) Distance Acquisition Demo

本项目基于 STM32G431RBT6，演示如何使用 **UART + DMA + ReceiveToIdle** 模式接收 TOF 传感器串口数据，并解析测距结果，通过 LCD 显示距离值。

TOF 模块输出的原始数据格式如下：



State:0 , Range Valid
d: 113 mm


本工程实现对以上字符串数据进行实时解析，提取距离值并在 LCD 上显示。

---

## 1. 功能说明

本项目主要功能包括：

- 使用 **UART2 + DMA** 非阻塞方式接收 TOF 串口数据  
- 通过 `RxEventCallback` 处理接收到的数据包  
- 解析 TOF 模块输出的字符串格式（如 State、距离 d:xxx）  
- 将有效的距离数据显示在 LCD 上  
- 工程结构清晰，可直接用于 TOF 测距类项目

---

## 2. 软件架构



/Core
├── Inc/
├── Src/
├── my_main/
├── my_main.c # 用户主逻辑
├── my_main.h
├── lcd/
├── lcd.c # LCD 驱动
├── lcd.h


关键模块说明：

- **uart_serv()**：解析 TOF 字符串数据  
- **HAL_UARTEx_RxEventCallback()**：UART + DMA 空闲中断回调，负责数据搬运与解析  
- **loop()**：实时刷新 LCD 显示  
- **setup()**：LCD 初始化 + UART DMA 初始化  

---

## 3. 硬件环境

- **MCU**：STM32G431RBT6  
- **LCD**：支持 `LCD_DisplayStringLine()` 接口的 TFT/OLED 等  
- **TOF 模块**：输出串口格式为：



State:0 , Range Valid
d: 113 mm


- **串口连接**

| TOF | STM32 |
|-----|--------|
| TX  | PA3 (USART2 RX) |
| GND | GND |
| 5V/3.3V | VCC |

UART 参数：

| 参数 | 配置 |
|------|-------|
| Baudrate | 115200 |
| Data bits | 8 |
| Stop bits | 1 |
| Parity | None |

---

## 4. TOF 串口数据解析流程

TOF 模块每次发送两行文本：



State:0 , Range Valid
d: 113 mm


解析流程：

1. DMA 持续接收数据  
2. 发生 **IDLE** 中断 → 触发 `RxEventCallback`  
3. 将 DMA buffer 数据复制到 rxdata  
4. 调用 `uart_serv()` 解析字符串内容  
5. 更新全局变量 `dist`  
6. 在主循环中刷新 LCD 显示  

---

## 5. 核心代码说明

### 5.1 ReceiveToIdle 接收初始化

```c
HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxbuff, sizeof(rxbuff));
__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

5.2 回调函数处理 DMA 数据
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
        memcpy(rxdata, rxbuff, Size);
        rxdata[Size] = '\0';

        uart_serv(rxdata);
        memset(rxbuff, 0, sizeof(rxbuff));

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxbuff, sizeof(rxbuff));
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }
}

5.3 TOF 字符串解析函数
void uart_serv(char *str)
{
    int state = -1;
    int distance = -1;

    char *s = strstr(str, "State:");
    if (s != NULL) sscanf(s, "State:%d", &state);

    char *p = strstr(str, "d:");
    if (p != NULL) sscanf(p, "d: %d", &distance);

    if (state == 0 && distance > 0)
    {
        dist = distance;
    }
}

5.4 LCD 显示距离
sprintf(str_lcd, "Distance: %03d mm", dist);
LCD_DisplayStringLine(Line3, str_lcd);

6. 如何编译运行

使用 STM32CubeIDE 打开工程

检查串口与 DMA 配置是否一致

编译并下载到 STM32

打开串口调试工具可观察 Debug 输出

LCD 上将显示实时距离，例如：

Distance: 113 mm

7. 后续可扩展方向

添加均值滤波或卡尔曼滤波

将 TOF 集成进移动小车避障控制

增加多 TOF 传感器同时解析

支持 VL6180X、VL53L0X、VL53L1X 等更多型号

使用 FreeRTOS 进行任务化重构
