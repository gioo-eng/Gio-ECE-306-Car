//******************************************************************************
//
//  Description: This file contains the Function prototypes
//
//  Jim Carlson
//  Aug 2013
//  Built with IAR Embedded Workbench Version: V4.10A/W32 (5.40.1)
//******************************************************************************
// Functions

// Main
void main(void);

// Initialization
void Init_Conditions(void);
void Init_DAC(void);

// Interrupts
void enable_interrupts(void);
__interrupt void Timer0_B0_ISR(void);
__interrupt void switch_interrupt(void);

// Analog to Digital Converter
void detect(void);
void Init_ADC(void);
void black_flag(void);

// Clocks
void Init_Clocks(void);

// LED Configurations
void Init_LEDs(void);
void IR_LED_control(char selection);
void Backlite_control(char selection);

  // LCD
void Display_Process(void);
void Display_Update(char p_L1,char p_L2,char p_L3,char p_L4);
void enable_display_update(void);
void update_string(char *string_data, int string);
void Init_LCD(void);
void lcd_clear(void);
void lcd_putc(char c);
void lcd_puts(char *s);

void lcd_power_on(void);
void lcd_write_line1(void);
void lcd_write_line2(void);
//void lcd_draw_time_page(void);
//void lcd_power_off(void);
void lcd_enter_sleep(void);
void lcd_exit_sleep(void);
//void lcd_write(unsigned char c);
//void out_lcd(unsigned char c);

void Write_LCD_Ins(char instruction);
void Write_LCD_Data(char data);
void ClrDisplay(void);
void ClrDisplay_Buffer_0(void);
void ClrDisplay_Buffer_1(void);
void ClrDisplay_Buffer_2(void);
void ClrDisplay_Buffer_3(void);

void SetPostion(char pos);
void DisplayOnOff(char data);
void lcd_BIG_mid(void);
void lcd_BIG_bot(void);
void lcd_120(void);

void lcd_4line(void);
void lcd_out(char *s, char line, char position);
void lcd_rotate(char view);

//void lcd_write(char data, char command);
void lcd_write(unsigned char c);
void lcd_write_line1(void);
void lcd_write_line2(void);
void lcd_write_line3(void);

void lcd_command( char data);
void LCD_test(void);
void LCD_iot_meassage_print(int nema_index);

// Menu
void Menu_Process(void);

// Ports
void Init_Ports(void);
void Init_Port_1(void);
void Init_Port_2(void);

//void Init_Port3(char smclk);
void Init_Port_3(void);
void Init_Port_4(void);
void Init_Port_5(void);
void Init_Port_6(void);

// SPI
void Init_SPI_B1(void);
void SPI_B1_write(char byte);
void spi_rs_data(void);
void spi_rs_command(void);
void spi_LCD_idle(void);
void spi_LCD_active(void);
void SPI_test(void);
void WriteIns(char instruction);
void WriteData(char data);

// Switches
void Init_Switches(void);
void switch_control(void);
void enable_switch_SW1(void);
void enable_switch_SW2(void);
void disable_switch_SW1(void);
void disable_switch_SW2(void);
void Switches_Process(void);
void Init_Switch(void);
void Switch_Process(void);
void Switch1_Process(void);
void Switch2_Process(void);
void menu_act(void);
void menu_select(void);
void Switch_mode(void);
void Switch_mode_2(void);

// Timers
void Init_Timers(void);
void Init_Timer_B0(void);
void Init_Timer_B1(void);
void Init_Timer_B2(void);
void Init_Timer_B3(void);
void delay (int);
void small_delay (int);

void usleep(unsigned int usec);
void usleep10(unsigned int usec);
void five_msec_sleep(unsigned int msec);
void measure_delay(void);
void out_control_words(void);

//Shapes
void Run_Circle(void);
void Run_Figure_8(void);
void Run_Triangle(void);
void project_5(void);

//----------------------------
//PWM speed movement calls
//----------------------------
void no_movement(void);

//slow movements
void slow_forward(void);
void slow_reverse(void);
void slow_left(void);
void slow_right(void);

//medium speed movements
void medium_forward(void);
void medium_reverse(void);
void medium_left(void);
void medium_right(void);

//fast speed movements
void fast_forward(void);
void fast_reverse(void);
void fast_left(void);
void fast_right(void);

// lucid speed movements
void lucid_forward(void);
void lucid_reverse(void);
void lucid_left(void);
void lucid_right(void);

void project_7(void);

//Serial communication
void Init_Serial_UCA0(int speed);
void serial_update(void);
void rx_process_usb(void);
void rx_process_iot(void);
void IOT_Process(void);
void IOT_State(void);
void initial_process_iot(void);


