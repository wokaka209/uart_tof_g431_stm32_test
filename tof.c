
#include "tof.h"





/**
 * @brief 微秒级延时函数（基于DWT）
 * @param us: 延时微秒数
 */
static void delay_us(uint32_t us)
{
    uint32_t t = us * 80;  // 80 为大约系数（适合 G4 170MHz）
    while(t--) __NOP();
}




void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PA6 SDA */
    GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SOFT_I2C_SDA_GPIO, &GPIO_InitStruct);

    /* PA7 SCL */
    GPIO_InitStruct.Pin = SOFT_I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(SOFT_I2C_SCL_GPIO, &GPIO_InitStruct);

    SDA_H();
    SCL_H();
}

void Soft_I2C_Start(void)
{
    SDA_OUT();
    SDA_H();
    SCL_H();
    delay_us(5);
    SDA_L();
    delay_us(5);
    SCL_L();
}

void Soft_I2C_Stop(void)
{
    SDA_OUT();
    SCL_L();
    SDA_L();
    delay_us(5);
    SCL_H();
    delay_us(5);
    SDA_H();
    delay_us(5);
}

uint8_t Soft_I2C_WriteByte(uint8_t dat)
{
    SDA_OUT();
    for(uint8_t i=0;i<8;i++)
    {
        (dat & 0x80) ? SDA_H() : SDA_L();
        dat <<= 1;
        delay_us(2);
        SCL_H();
        delay_us(5);
        SCL_L();
        delay_us(2);
    }

    /* Ack */
    SDA_IN();
    delay_us(2);
    SCL_H();
    delay_us(5);
    uint8_t ack = SDA_READ();
    SCL_L();
    SDA_OUT();

    return (ack == 0);
}

uint8_t Soft_I2C_ReadByte(uint8_t ack)
{
    uint8_t dat = 0;
    SDA_IN();

    for(uint8_t i=0;i<8;i++)
    {
        SCL_H();
        delay_us(5);
        dat <<= 1;
        if(SDA_READ()) dat |= 1;
        SCL_L();
        delay_us(5);
    }

    SDA_OUT();
    if(ack) SDA_L();
    else SDA_H();

    delay_us(2);
    SCL_H();
    delay_us(5);
    SCL_L();

    return dat;
}

static void i2c_write_reg16addr(uint8_t dev, uint16_t reg, uint8_t data)
{
    Soft_I2C_Start();
    Soft_I2C_WriteByte((dev << 1) | 0);
    Soft_I2C_WriteByte(reg >> 8);
    Soft_I2C_WriteByte(reg & 0xFF);
    Soft_I2C_WriteByte(data);
    Soft_I2C_Stop();
}

static void i2c_read_reg16addr(uint8_t dev, uint16_t reg, uint8_t *buf)
{
    Soft_I2C_Start();
    Soft_I2C_WriteByte((dev << 1) | 0);
    Soft_I2C_WriteByte(reg >> 8);
    Soft_I2C_WriteByte(reg & 0xFF);

    Soft_I2C_Start();
    Soft_I2C_WriteByte((dev << 1) | 1);
    *buf = Soft_I2C_ReadByte(0);
    Soft_I2C_Stop();
}

uint8_t VL6180X_WriteReg8(uint16_t reg, uint8_t val)
{
    i2c_write_reg16addr(VL6180X_ADDR, reg, val);
    return 1;
}

uint8_t VL6180X_ReadReg8(uint16_t reg, uint8_t *val)
{
    i2c_read_reg16addr(VL6180X_ADDR, reg, val);
    return 1;
}

uint8_t VL6180X_Init(void)
{
    uint8_t id = 0;
    VL6180X_ReadReg8(0x000, &id);
    if(id != 0xB4) return 0;   // 芯片 ID 错误

    // 清中断
    VL6180X_WriteReg8(0x015, 0x07);

    // 设置 ALS 积分时间为 100ms（0x63）
    VL6180X_WriteReg8(0x040, 0x63);

    return 1;
}

uint8_t VL6180X_ReadRange(uint8_t *range_mm)
{
    uint8_t status = 0;
    uint8_t range = 0;
    uint16_t timeout = 200; // 最多等 200 次（约 20ms）

    // 1. 启动单次测距
    VL6180X_WriteReg8(0x018, 0x01);

    // 2. 等待测距完成（INTERRUPT_STATUS bit2 = 1）
    while(timeout--)
    {
        VL6180X_ReadReg8(0x04F, &status);
        if (status & 0x04)  // bit2 = New Sample Ready
            break;
    }

    if(timeout == 0)
        return 2;   // timeout，表示一直没准备好

    // 3. 读取测距结果
    VL6180X_ReadReg8(0x062, &range);

    // 4. 清中断
    VL6180X_WriteReg8(0x015, 0x07);

    *range_mm = range;

    return 1;   // 成功
}