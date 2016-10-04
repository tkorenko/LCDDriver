#include <stdint.h>
extern "C" {
#include "LCDPort.h"
#include "MockPeriphIO.h"
};
#include "LCDPortSpy.h"

static int rs_line_state;
static int rw_line_state;
static int ce_line_state;
static int lcd_port_data_direction;
static int lcd_port_data_width;

void
LCDPortSpy_ResetToDefaultState()
{
    rs_line_state = LINE_STATE_UNDEFINED;
    rw_line_state = LINE_STATE_UNDEFINED;
    ce_line_state = LINE_STATE_UNDEFINED;
    lcd_port_data_direction = LCD_PORT_DATA_DIR_UNDEFINED;
    lcd_port_data_width = LCD_PORT_DATA_WIDTH_UNDEFINED;
}

extern "C" void
LCDPort_Init(int32_t dataWidth)
{
    ce_line_state = LINE_STATE_DEASSERTED;
    rs_line_state = LINE_STATE_DEASSERTED;
    rw_line_state = LINE_STATE_ASSERTED;
    lcd_port_data_direction = LCD_PORT_DATA_DIR_INPUT8;
    lcd_port_data_width = dataWidth;
}

extern "C" void
LCDPort_Deinit(void)
{
    LCDPortSpy_ResetToDefaultState();
}

static void
setPort_dataDirection(int32_t addr, int32_t d)
{
    MockPeriphIO_Write(addr, d);
    lcd_port_data_direction = d;
}

extern "C" void
LCDPort_SetDirection_Input8(void)
{
    setPort_dataDirection(LCD_FAKE_DIRECTION8_REG, LCD_PORT_DATA_DIR_INPUT8);
}

extern "C" void
LCDPort_SetDirection_Output8(void)
{
    setPort_dataDirection(LCD_FAKE_DIRECTION8_REG, LCD_PORT_DATA_DIR_OUTPUT8);
}

extern "C" void
LCDPort_SetDirection_Input4(void)
{
    setPort_dataDirection(LCD_FAKE_DIRECTION4_REG, LCD_PORT_DATA_DIR_INPUT4);
}

extern "C" void
LCDPort_SetDirection_Output4(void)
{
    setPort_dataDirection(LCD_FAKE_DIRECTION4_REG, LCD_PORT_DATA_DIR_OUTPUT4);
}

extern "C" void
LCDPort_Out8(int32_t n)
{
    MockPeriphIO_Write(LCD_DATA8_ADDR, n);
}

extern "C" void
LCDPort_Out4(int32_t n)
{
    MockPeriphIO_Write(LCD_DATA4_ADDR, n);
}

extern "C" int32_t
LCDPort_In8(void)
{
    return MockPeriphIO_Read(LCD_DATA8_ADDR);
}

extern "C" int32_t
LCDPort_In4(void)
{
    return MockPeriphIO_Read(LCD_DATA4_ADDR);
}

static void
configureLine_RS(int v)
{
    MockPeriphIO_Write(LCD_RS_ADDR, v);
    rs_line_state = v;
}

extern "C" void
LCDPort_SetRS(void)
{
    configureLine_RS(LINE_STATE_ASSERTED);
}

extern "C" void
LCDPort_ClearRS(void)
{
    configureLine_RS(LINE_STATE_DEASSERTED);
}

static void
configureLine_RW(int v)
{
    MockPeriphIO_Write(LCD_RW_ADDR, v);
    rw_line_state = v;
}


extern "C" void
LCDPort_SetRW(void)
{
    configureLine_RW(LINE_STATE_ASSERTED);
}

extern "C" void
LCDPort_ClearRW(void)
{
    configureLine_RW(LINE_STATE_DEASSERTED);
}

static void
configureLine_CE(int v)
{
    MockPeriphIO_Write(LCD_CE_ADDR, v);
    ce_line_state = v;
}

extern "C" void
LCDPort_SetCE(void)
{
    configureLine_CE(LINE_STATE_ASSERTED);
    
}

extern "C" void
LCDPort_ClearCE(void)
{
    configureLine_CE(LINE_STATE_DEASSERTED);
}

int
LCDPortSpy_GetDataDirection()
{
    return lcd_port_data_direction;
}

int
LCDPortSpy_GetRS()
{
    return rs_line_state;
}

int
LCDPortSpy_GetRW()
{
    return rw_line_state;
}

int
LCDPortSpy_GetCE()
{
    return ce_line_state;
}

