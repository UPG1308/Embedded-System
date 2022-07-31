
#include <xc.h>
#include "clcd.h"
#include "main.h"
#include "matrix_keypad.h"
#include "timers.h"

#pragma config WDTE = OFF

unsigned char operation_mode , reset_flag, convection_mode_op;
unsigned char flag = 0;
char sec = 0, min = 0, temp = 0;
static void init_config(void) {
    //Initialization of CLCD Module
    init_clcd();
    
    //Initialization of Matrix Keypad Module
    init_matrix_keypad();
    
    //Initialization of Timer2 Module
    init_timer2();
    
    PEIE = 1;
    GIE = 1;
    FAN_DDR = 0;           // Fan pin setting as output
    FAN = OFF;               //Fan is kept OFF 
    
    BUZZER_DDR = 0;
    BUZZER = OFF;
    DOOR_DDR = 1;    //Input
}

void main(void) {
    unsigned char key;
     // set the operation
    init_config();
    power_on_screen();
    clear_clcd();
    operation_mode = COOKING_MODE_DISPLAY;
    while (1) 
    {
        key = read_matrix_keypad(STATE);
        if((operation_mode == MICRO_MODE) || (operation_mode == GRILL_MODE ) || (operation_mode == CONVECTION_MODE))
        {
            ;
        }
        else if(key == 1 && (operation_mode != TIME_DISPLAY)) //MKP key 1 is pressed 
        {
            operation_mode = MICRO_MODE;
            reset_flag = MODE_RESET;
            clear_clcd();
            clcd_print(" Power = 900W ", LINE2(0));
            __delay_ms(3000);
            clear_clcd();
        }
        else if(key == 2 && (operation_mode != TIME_DISPLAY))
        {
            operation_mode = GRILL_MODE;
            reset_flag = MODE_RESET;
            clear_clcd();
        }
        else if(key == 3)
        {
            operation_mode = CONVECTION_MODE;
            convection_mode_op = SET_TEMP;
            reset_flag = RESET_TEMP;
            clear_clcd();
        }
        else if(key == 4)
        {
            if(operation_mode == COOKING_MODE_DISPLAY)
            {
                min = 0;
                sec = 30;
                TMR2ON = ON;
                FAN = ON;
                clear_clcd();
                operation_mode = TIME_DISPLAY;
            }
            else if(operation_mode == TIME_DISPLAY)
            {
                sec = sec + 30;
                if(sec >= 60 )
                {
                    min++;
                    sec = sec - 60;
                }
            }
            else if(operation_mode == PAUSE)
            {
                TMR2ON = ON;
                FAN = ON;
                operation_mode = START;
            }
        }
        else if(key == 5)
        {
            operation_mode = PAUSE;
        }
        else if(key == 6)             // Stop the current task
        {
            operation_mode = STOP;
            clear_clcd();
        }
        switch(operation_mode)
        {
            case COOKING_MODE_DISPLAY: 
                cooking_mode_display();
                break;
            
            case MICRO_MODE:
                set_time(key);
                break;
            case GRILL_MODE:
                set_time(key);
                break;
            case CONVECTION_MODE:
                if(convection_mode_op == SET_TEMP)    // to call set_temp function
                {
                    set_temp(key);
                    if(convection_mode_op == SET_TIME)
                    {
                        continue;
                    }
                }
                else if(convection_mode_op == SET_TIME)
                {
                    set_time(key);
                    
                }
                break;
                
            case START :
            
            case TIME_DISPLAY :
                time_display_screen();
                break;
                
            case STOP:
                TMR2ON = OFF;
                FAN = OFF;
                operation_mode = COOKING_MODE_DISPLAY;
                break;
        
            case PAUSE:
                TMR2ON = OFF;
                FAN = OFF;
                break;
        }
       
        reset_flag = RESET_NOTHING;
    }
    return;
}

void clear_clcd(void)
{
     clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
     __delay_us(100);
}

void power_on_screen(void)
{
    for(int i = 0; i<16; i++)
    {
         clcd_putch(BLOCK, LINE1(i));
    }
   
    clcd_print("  Powering ON   ",LINE2(0));
    clcd_print(" Microwave Oven ",LINE3(0));
    for(int i=0; i<16; i++)
    {
         clcd_putch(BLOCK, LINE4(i));
    }
    __delay_ms(3000);
}

void cooking_mode_display(void)
{
    clcd_print("1.Micro",LINE1(0));
    clcd_print("2.Grill",LINE2(0));
    clcd_print("3.Convection",LINE3(0));
    clcd_print("4.Start",LINE4(0));
}

void set_time(unsigned char key)
{
    static int wait , blink, key_count, blink_pos;
    if(reset_flag == MODE_RESET)
    {
        wait = 15;
        blink = 0;
        key_count = 0;
        sec = 0;
        min = 0;
        blink_pos = 0;
        key = ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)",LINE1(0));
        clcd_print("TIME- ",LINE2(0));
        clcd_putch(':',LINE2(8));
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
    
    
    if(key != '*' && key != '#' && key != ALL_RELEASED)
    {
        key_count++;
        if(key_count <= 2)
        {
            sec = sec * 10 + key;
            blink_pos = 0;
        }
        else if(key_count > 2 && key_count <= 4)
        {
            min = min * 10 + key;
            blink_pos = 1;
        }
        
    }
    
    if(key == '*')
    {
        if(key_count <= 2)
        {
            sec = 0;
            key_count = 0;
        }
        else if(key_count > 2 && key_count <= 4)
        {
            min = 0;
            key_count = 2;
        }
        
    }
    if(key == '#')
    {
        clear_clcd();
        operation_mode = TIME_DISPLAY;
        FAN = 1;          // Turn ON Fan
        /* Switching ON the Timer2*/
        TMR2ON = 1;
    }
    //Display number of mins, secs
    if(wait++ == 15)
    {
        wait = 0;
        blink = !blink;
        clcd_putch(sec/10 + '0', LINE2(9));
        clcd_putch(sec%10 + '0', LINE2(10));
    
    //Display number of mins
        clcd_putch(min/10 + '0', LINE2(6));
        clcd_putch(min%10 + '0', LINE2(7));
    }
    if(blink)
    {
        switch(blink_pos)
        {
            case 0 : clcd_print("  ",LINE2(9));  // To blink sec
                     break;
                     
            case 1 : clcd_print("  ",LINE2(6));  // To blink min
                     break;
        }
        
       
    }
  
}

void door_status_check(void)
{
    //if RB3 is pressed door will be open
    if(DOOR == OPEN)       // RB3 is 0
    {
        TMR2ON = OFF;
        FAN = OFF;
        BUZZER = ON;
        clear_clcd();
        clcd_print("DOOR STATUS:OPEN", LINE2(0));
        clcd_print("PLEASE CLOSE!",LINE3(0));
        
        while(DOOR == OPEN)
        {
            ;
        }
        BUZZER = OFF;
        TMR2ON = ON;
        FAN = ON;
        clear_clcd();
    }
}

void time_display_screen(void)
{
    door_status_check();
    clcd_print(" TIME =  ", LINE1(0));
    clcd_putch(min/10 + '0', LINE1(9));
    clcd_putch(min%10 + '0', LINE1(10));
    clcd_putch(':',LINE1(11));
    
    clcd_putch(sec/10 + '0', LINE1(12));
    clcd_putch(sec%10 + '0', LINE1(13));
    
    
    clcd_print("4.START / RESUME", LINE2(0));
    clcd_print("5.PAUSE", LINE3(0));
    clcd_print("6.STOP", LINE4(0));
    
    if((sec == 0) && (min == 0))
    {
        clear_clcd();
        clcd_print("Time Up",LINE2(0));
        TMR2ON = OFF;         //Turn OFF Timer
        FAN = OFF;  
        BUZZER = ON;
        __delay_ms(3000);
        BUZZER = OFF;
        clear_clcd();
        operation_mode = COOKING_MODE_DISPLAY;
        
        
    }
}

void set_temp(unsigned char key)
{
    static char temp_arr[4];
    static int wait, blink, key_count;
    if(reset_flag == RESET_TEMP)
    {
        key = ALL_RELEASED;
        temp = 0;
        wait = 15;
        blink = 0;
        key_count = 0;
        clcd_print("SET TEMP. ( C)",LINE1(0));
        clcd_putch(DEGREE,LINE1(10));
        clcd_print(" TEMP :",LINE2(0));
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
    //Read Temperature
    if(key != '*' && key != '#' && key != ALL_RELEASED)
    {
        key_count++;
        if(key_count <= 3)
        {
            temp = temp * 10 + key;
            
        }
        
       
    }
    else if(key == '*')
        {
            temp = 0;
            key_count = 0;
        }
        else if(key == '#')
        {
            clear_clcd();
            clcd_print("  Pre-Heating   ",LINE1(0));
            clcd_print("Time Rem.=",LINE3(0));
            
            //Set pre_heating time for 3 min
            sec = 180;
            TMR2ON = ON;
            while(sec != 0)
            {
                  clcd_putch(sec/100 + '0', LINE3(10));
                  clcd_putch((sec/10)%10 + '0', LINE3(11));
                  clcd_putch(sec % 10 + '0', LINE3(12));
            }
            
            //After pre_heating
            TMR2ON = OFF;
            // return SUCCESS;
            clear_clcd();
            convection_mode_op = SET_TIME;
            reset_flag = MODE_RESET;
            
        }
     // display temp
        
        temp_arr[0] = (temp/100) + '0';
        temp_arr[1] = (temp/10) % 10 + '0';
        temp_arr[2] = (temp % 10) + '0';
        temp_arr[3] = '\0';
        
        if(wait++ == 15)
        {
            wait = 0;
            blink = !blink;
            clcd_print(temp_arr,LINE2(8));
        }
        if(blink)
        {
            clcd_print("   ",LINE2(8));


        }
        
      //  return 0x11;
    
}
