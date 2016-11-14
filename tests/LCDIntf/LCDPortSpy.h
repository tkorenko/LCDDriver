#ifndef D_LCDPortSpy_h
#define D_LCDPortSpy_h

enum {
    LCD_PORT_DATA_DIR_UNDEFINED = 0,
    LCD_PORT_DATA_DIR_INPUT4,
    LCD_PORT_DATA_DIR_OUTPUT4,
    LCD_PORT_DATA_DIR_INPUT8,
    LCD_PORT_DATA_DIR_OUTPUT8,
};

enum {
    LINE_STATE_UNDEFINED = -1,
    LINE_STATE_DEASSERTED,
    LINE_STATE_ASSERTED
};

// no relation with real hardware 
enum {
    LCD_FAKE_DIRECTION4_REG = 0x3FFFFFF0,
    LCD_FAKE_DIRECTION8_REG,
    LCD_DATA4_ADDR          = 0x40001000,
    LCD_DATA8_ADDR,
    LCD_RS_ADDR             = 0x42001000,
    LCD_RW_ADDR,
    LCD_CE_ADDR,
};

void LCDPortSpy_ResetToDefaultState();
int LCDPortSpy_GetDataDirection();
int LCDPortSpy_GetRS();
int LCDPortSpy_GetRW();
int LCDPortSpy_GetCE();

#endif /* #ifndef D_LCDPortSpy_h */
