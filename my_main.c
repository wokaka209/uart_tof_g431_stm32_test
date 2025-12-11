/*
 * @Author: wokaka209 1325536985@qq.com
 * @Date: 2025-12-08 20:30:44
 * @LastEditors: wokaka209 1325536985@qq.com
 * @LastEditTime: 2025-12-11 22:55:46
 * @FilePath: \eided:\vscodeied\stm32g431RBT6\tof_test\my_mian\my_main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "my_main.h"

char str[] = "State:0 , Range Valid\nd: 113 mm";

/******lcd 变量***** */
uint8_t str_lcd[20];


/*tof 变量*/
int LLLL[3] = 0;
uint8_t dist = 0;

/***************uart******************/
char rxbuff[100] = {0};
char txbuff[100] = {0};
#define rxlen 100
char rxdata[rxlen] = {0};
extern DMA_HandleTypeDef hdma_usart2_rx;

void setup()
{
    LCD_Init();
    LCD_SetBackColor(Blue);
    LCD_SetTextColor(White);
    LCD_Clear(Blue);
    // Soft_I2C_Init();
    // VL6180X_Init();
    // HAL_Delay(5);
    // if(!VL6180X_Init())
    // {
    //     sprintf(str_lcd,"VL6180X Init Failed");
    //     LCD_DisplayStringLine(Line1,str_lcd);
    //     memset(str_lcd,0,20);
    //     while(1);
    // }
    // HAL_Delay(5);
    // 使用DMA以非阻塞方式接收UART数据直到空闲
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxbuff, sizeof(rxbuff));
    // 禁用DMA半传输中断
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

void loop()
{
        sprintf(str_lcd, "Distance: %03d mm", dist);
        LCD_DisplayStringLine(Line3, str_lcd);
        memset(str_lcd, 0, 20);
//     if (VL6180X_ReadRange(&dist))
//     {
//         sprintf(str_lcd, "Distance: %d mm", dist);
//         LCD_DisplayStringLine(Line3, str_lcd);
//         memset(str_lcd, 0, 20);
//     }

//     else
//     {
//         sprintf(str_lcd, "error");
//         LCD_DisplayStringLine(Line4, str_lcd);
//         memset(str_lcd, 0, 20);

//         HAL_Delay(200);
//     }

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    UNUSED(Size);
    // 处理USART1的接收数据
    if (huart->Instance == USART2)
    {
        // 检查接收到的数据大小是否等于预期长度
        if (Size>0)
        {
            memcpy(rxdata, rxbuff, Size);
            rxdata[Size] = '\0';  // 添加字符串结束符
            uart_serv(rxdata);
            memset(rxbuff, 0, sizeof(rxbuff));
        }
        // 准备接收下一个数据包
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxbuff, sizeof(rxbuff));
        // 禁用DMA中断
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }
}

// rxdata
void uart_serv(char *str)
{
    int state = -1;
    int distance = -1;

    // 解析 State 行
    char *s = strstr(str, "State:");
    if (s != NULL)
    {
        sscanf(s, "State:%d", &state);
    }

    // 解析 d: 行
    char *p = strstr(str, "d:");
    if (p != NULL)
    {
        sscanf(p, "d: %d", &distance);
    }

    // 判断有效
    
        dist = (uint16_t)distance;
}
