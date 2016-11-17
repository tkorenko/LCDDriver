#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "LCDIntf.h"
#include "LCDPort.h"

/*   The purpose of this implementation is to verify LCDIntf work with
 * real hardware.  WH1602 (HD44780) is connected to the STM32vldiscovery
 * board.
 *
 *   Its hard to find contiguously allocated 8 free GPIO lines
 * on VLDiscovery board, thus I gonna use the following mapping:
 *   PORTA[1]     -> LCD RS
 *   PORTA[2]     -> LCD RW
 *   PORTA[3]     -> LCD CE
 *   PORTA[4..7]  -> LCD D[0..3]  // Low  nibble
 *   PORTA[8..11] -> LCD D[4..7]  // High nibble
 *
 * (PORTA[0] belongs to VLDiscovery board -- User Button)
 *
 */

enum {
    PORT_RS_BIT = 0x2,
    PORT_RW_BIT = 0x4,
    PORT_CE_BIT = 0x8,
};

static void setControlLine_portMode(int32_t mode);
static void setInputModeOfDataLines(void);

void
LCDPort_Init(int32_t lcdPortDataWidth)
{
    // Enable Clock of PORTA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    LCDPort_ClearCE();
    LCDPort_ClearRS();
    LCDPort_SetRW();

    setControlLine_portMode(GPIO_Mode_Out_PP);

    setInputModeOfDataLines();
}

void
LCDPort_Deinit(void)
{
    setControlLine_portMode(GPIO_Mode_IN_FLOATING);

    setInputModeOfDataLines();
}

void
LCDPort_Out4(int32_t v)
{
    uint32_t keepControlLines = GPIOA->ODR & 0x0E;
    GPIOA->ODR = ((v & 0x0F) << 8) | keepControlLines;
}

void
LCDPort_Out8(int32_t v)
{
    uint32_t keepControlLines = GPIOA->ODR & 0x0E;
    GPIOA->ODR = ((v & 0xFF) << 4) | keepControlLines;
}

int32_t
LCDPort_In4(void)
{
    return ((GPIOA->IDR & 0x0F00) >> 8);
}

int32_t
LCDPort_In8(void)
{
    return ((GPIOA->IDR & 0x0FF0) >> 4);
}

void
LCDPort_SetDirection_Input8(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Setup PORTA[4..11] (data lines of LCD)
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5  | GPIO_Pin_6
        | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void
LCDPort_SetDirection_Output8(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Setup PORTA[4..11] (data lines of LCD)
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5  | GPIO_Pin_6
        | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void
LCDPort_SetDirection_Input4(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Setup PORTA[8..11] (data lines of LCD)
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9
        | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void
LCDPort_SetDirection_Output4(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Setup PORTA[8..11] (data lines of LCD)
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9
        | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void
LCDPort_SetRS(void)
{
    GPIOA->BSRR = PORT_RS_BIT;
}

void
LCDPort_ClearRS(void)
{
    GPIOA->BSRR = PORT_RS_BIT << 16;
}

void
LCDPort_SetRW(void)
{
    GPIOA->BSRR = PORT_RW_BIT;
}

void
LCDPort_ClearRW(void)
{
    GPIOA->BSRR = PORT_RW_BIT << 16;
}

void
LCDPort_SetCE(void)
{
    GPIOA->BSRR = PORT_CE_BIT;
}

void
LCDPort_ClearCE(void)
{
    GPIOA->BSRR = PORT_CE_BIT << 16;
}

static void
setControlLine_portMode(int32_t mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1  | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void
setInputModeOfDataLines(void)
{
    int32_t dataWidth = LCDIntf_GetPortDataWidth();
    if (LCD_PORT_DATA_WIDTH_8_BIT == dataWidth) {
        LCDPort_SetDirection_Input8();
    } else if (LCD_PORT_DATA_WIDTH_4_BIT == dataWidth) {
        LCDPort_SetDirection_Input4();
    }
}

