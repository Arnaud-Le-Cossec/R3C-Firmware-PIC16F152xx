/*
 * File:   main.c
 * Author: Arnaud LE COSSEC
 *
 * Created on 17 November 2023, 17:23
 */

// PIC16F15214 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator Mode Selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINTOSC_1MHZ// Power-up Default Value for COSC bits (HFINTOSC (1 MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; I/O function on RA4)
#pragma config VDDAR = HI       // VDD Range Analog Calibration Selection bit (Internal analog systems are calibrated for operation between VDD = 2.3V - 5.5V)

// CONFIG2
#pragma config MCLRE = EXTMCLR  // Master Clear Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RA3 pin function is MCLR)
#pragma config PWRTS = PWRT_OFF // Power-up Timer Selection bits (PWRT is disabled)
#pragma config WDTE = ON        // WDT Operating Mode bits (WDT enabled regardless of Sleep; SEN bit is ignored)
#pragma config BOREN = ON       // Brown-out Reset Enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection bit (Brown-out Reset Voltage (VBOR) set to 1.9V)
#pragma config PPS1WAY = ON     // PPSLOCKED One-Way Set Enable bit (The PPSLOCKED bit can be set once after an unlocking sequence is executed; once PPSLOCKED is set, all future changes to PPS registers are prevented)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block is disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF is disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block is not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block is not write-protected)
#pragma config WRTC = OFF       // Configuration Registers Write Protection bit (Configuration Registers are not write-protected)
#pragma config WRTSAF = OFF     // Storage Area Flash (SAF) Write Protection bit (SAF is not write-protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR. MCLRE Configuration bit is ignored.)

// CONFIG5
#pragma config CP = OFF         // User Program Flash Memory Code Protection bit (User Program Flash Memory code protection is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>
#include <pic16f15214.h>

#define FIRMWARE_VER 2

#define _XTAL_FREQ 1000000  // 1Mhz configuration de la carte

#define RX_BUFFER_SIZE 80
uint8_t RX_buffer[RX_BUFFER_SIZE];
uint8_t RX_index = 0;

#include "watchdog_driver.h"
#include "eusart_driver.h"
#include "i2c_driver.h"
#include "analog_driver.h"
#include "lora_driver.h"
#include "remote_mode_driver.h"

#define LED_PIN 0

/*DEFINE THE DATA TRANSMISSION INTERVAL
 * 1. Go the file 'watchdog_driver.h' and select the desired watchdog period.
 * 2. Set SLEEP_COUNTER_THRESHOLD_DEFAULT to the desired multiple of the watchdog period
 * 
 * The resulting transmission interval will be t = SLEEP_COUNTER_THRESHOLD_DEFAULT*(watchdog period)
 * Notes :
 *      This value should not be less than 1 minute
 *      The maximum LoRa join time should not exceed one watchdog period (see lora_driver.h)
 */
#define SLEEP_COUNTER_THRESHOLD_DEFAULT 1//14 for a full hour


/****************************************
 * INTERRUPT HANDLER
 ***************************************/
void __interrupt() ISR(void){

    if(PIR1bits.RC1IF){ // handle RX pin interrupts
        
        if(RX_index < RX_BUFFER_SIZE){
            RX_buffer[RX_index] = RC1REG;
            RX_index ++;
        }

        if(RC1STAbits.FERR){
            //PORTA ^= (1<<LED_PIN);
            RC1STAbits.SPEN = 0;
            RC1STAbits.SPEN = 1;

        }
        if(RC1STAbits.OERR){
            //PORTA ^= (1<<LED_PIN);
            RC1STAbits.CREN = 0;
            RC1STAbits.CREN = 1;
        }
        
        //PIR3bits.RC2IF = 0;
    } // end RX pin interrupt handlers
    
    if(PIR1bits.SSP1IF){ // handle i2c in slave mode Check for SSPIF  
        /*
        if(SSP1STATbits.R_nW == 1){ // Master read (slave transmit)
            SSP1BUF = 0xF0 | FIRMWARE_VER; // Load array value
            SSP1CON1bits.CKP = 1; // Release clock stretch
        }*/
        PIR1bits.SSP1IF = 0; // Clear SSP interrupt flag
        PIR1bits.BCL1IF = 0; // Clear Bus Collision IF
    }

} // end ISRs*/

/****************************************
 * MAIN
 ***************************************/
void main(void) {
    
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    
    TRISA &= !(1<<LED_PIN);		// Set outputs

    ANSELA = 0x0;
    
    /*Check if we are in remote mode*/
    if(remote_check()){
        
        I2C_setup_slave(0x30);
        
        while(1){
            PORTA ^= (1<<LED_PIN);		// Set LED to 1
            __delay_ms(500);
        };
    }
    
    
    /*Setup I2C*/
    I2C_setup();/*

    
    /*Setup serial communication*/
    EUSART_setup();
    EUSART_clear_buffer(RX_buffer, RX_BUFFER_SIZE);
    
    /*Setup ADC*/
    Analog_setup();
    
    /*Setup LoRa chip*/
    //PORTA |= (1<<LED_PIN);		// Set LED to 1
    __delay_ms(1000);
    LoRa_setup();
    //PORTA &= !(1<<LED_PIN);		// Set LED to 0
    
    
    /*Setup watchdog*/
    WDT_setup();
    
    /*Declare sleep counter*/
    uint8_t sleep_counter = 0;
    
    //float temp;
    //float humidity;
    
    uint16_t rawtemp;
    uint16_t rawhum;
    uint8_t battery;
    
    while(1){
           
        /*Start sleep, will be awaken by watchdog*/
        AT_command("AT+LOWPOWER"); // sleep LoRa modem
        SLEEP_start(); // sleep ourselves
        
        /*SLEEPING ZzZzz*/
        
        /*The following code is executed after wakeup*/
        /*increment sleep counter*/
        sleep_counter++;
        
        if(sleep_counter >= SLEEP_COUNTER_THRESHOLD_DEFAULT){
            
            sleep_counter = 0;
            
            AT_command("Wake up !!");
            /*Test Gateway connection*/
            if(!AT_command_check("AT+JOIN", "+JOIN: Joined already", 21)){
                /*Test fail, reconnect to LoRa*/
                PORTA |= (1<<LED_PIN);		// Set LED to 1
                LoRa_setup();
                PORTA &= !(1<<LED_PIN);		// Set LED to 0
            }
            
            /*Measure sensors*/
            //I2C_SHT4x_read(&temp, &humidity);
            I2C_SHT4x_read_raw(&rawtemp, &rawhum);
            battery = Analog_read_percent();
            
            /*Send message*/
            //LoRa_send_data((uint16_t)temp*100.0, (uint16_t)humidity*100.0, battery);
            LoRa_send_data(rawtemp, rawhum, battery);
            
        }
    }
}