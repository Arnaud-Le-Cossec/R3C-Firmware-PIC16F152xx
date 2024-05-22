#define SHT40_DEVICE_ADDRESS 0x46

void I2C_setup(void);
void I2C_wait(void);
void I2C_start(void);
void I2C_RepeatedStart(void);
void I2C_stop(void);
void I2C_write(uint8_t data);
uint8_t I2C_read();
uint8_t I2C_write_query(uint8_t address, uint8_t data);
uint8_t I2C_read_query(uint8_t address, uint8_t *data, uint8_t number_of_bytes);
uint8_t I2C_SHT4x_read(float *t_degC, float *rh_pRH);
void I2C_MCP23008_write(uint8_t address, uint8_t reg_addr, uint8_t data);
void I2C_MCP23008_read(void);


void I2C_setup(void){

    /*Set desired I2C clock frequency (in kHz)*/
    uint8_t i2c_freq = 100;
    /*Set RA1 (SCL) and RA2 (SDA) as inputs*/
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    //ODCONAbits.ODCA1 = 1;
    //ODCONAbits.ODCA2 = 1;
    RA1PPS = 0x07; // SCL1 output PPS address (on PIC16F15214)
    RA2PPS = 0x08; // SDA1 output PPS address (on PIC16F15214)
    SSP1CLKPPS = 0x1; // Set SCL1 input PPS to RA1 (on PIC16F15214)
    SSP1DATPPS = 0x2; // Set SDA1 input PPS to RA2 (on PIC16F15214)
    /**/
    SSP1STATbits.SMP = 1; // Default settings + Standard speed
    /*Host Synchronous Serial Port Mode Select bits*/
    SSP1CON1bits.SSPM = 0b1000; // Master mode
    /**/
    SSP1CON2 = 0; // Default settings
    /*Speed*/
    SSP1ADD = 3;//24;//(_XTAL_FREQ/(4*i2c_freq*100))-1; //Setting Clock Speed;   //100kbit/s
    /*Host Synchronous Serial Port Enable bit*/
    SSP1CON1bits.SSPEN = 1; // Enable
    
    PIR1bits.BCL1IF = 0; // Clear Bus Collision IF
    PIR1bits.SSP1IF = 0; // Clear SSP interrupt flag
    PIE1bits.BCL1IE = 1; // Enable BCLIF
    PIE1bits.SSP1IE = 1; // Enable SSPIF

}

void I2C_setup_slave(uint8_t slaveAddress){
    /*Set RA1 (SCL) and RA2 (SDA) as inputs*/
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    ODCONAbits.ODCA1 = 1;
    ODCONAbits.ODCA2 = 1;
    RA1PPS = 0x07; // SCL1 output PPS address (on PIC16F15214)
    RA2PPS = 0x08; // SDA1 output PPS address (on PIC16F15214)
    SSP1CLKPPS = 0x1; // Set SCL1 input PPS to RA1 (on PIC16F15214)
    SSP1DATPPS = 0x2; // Set SDA1 input PPS to RA2 (on PIC16F15214)
    /**/
    SSP1STATbits.SMP = 1; // Default settings + Standard speed
    /*Host Synchronous Serial Port Mode Select bits*/
    SSP1CON1bits.SSPM = 0b0110; // 7-bit slave mode
    SSP1CON2bits.SEN = 1; // Enable clock stretching
    SSP1CON3bits.SBCDE = 1; // Enable BCLIF
    SSP1ADD = slaveAddress; // Load slave address
    SSP1CON1bits.SSPEN = 1; // Enable the module
    
    PIR1bits.BCL1IF = 0; // Clear Bus Collision IF
    PIR1bits.SSP1IF = 0; // Clear SSP interrupt flag
    PIE1bits.BCL1IE = 1; // Enable BCLIF
    PIE1bits.SSP1IE = 1; // Enable SSPIF
    INTCONbits.PEIE = 1; // Enable periph interrupts
    INTCONbits.GIE = 1; // Enable global interrupts
}

void I2C_wait(void){
   while ((SSP1STAT & 0b00000100) || (SSP1CON2 & 0x00011111)); //Check if busy
}

void I2C_start(void){
  I2C_wait();    
  SSP1CON2bits.SEN = 1;             //Initiate start condition
}

void I2C_RepeatedStart(void){
  I2C_wait();
  SSP1CON2bits.RSEN = 1;           //Initiate repeated start condition
}

void I2C_stop(void){
  I2C_wait();
  SSP1CON2bits.PEN = 1;           //Initiate stop condition
}

void I2C_write(uint8_t data){
  I2C_wait();
  SSP1BUF = data;         //Write data to SSP1BUF
}

uint8_t I2C_read(){
  uint8_t tmp;
  I2C_wait();
  SSP1CON2bits.RCEN = 1;
  I2C_wait();
  while(PIR1bits.SSP1IF == 1); // wait SSP interrupt flag
  tmp = SSP1BUF;      //Read data from SSP1BUF
  I2C_wait();
  SSP1CON2bits.ACKDT = 0;    //Acknowledge bit
  SSP1CON2bits.ACKEN = 1;          //Acknowledge sequence
  return tmp;
}

uint8_t I2C_write_query(uint8_t address, uint8_t data){
    I2C_start();         //Start condition
    address = (address << 1)&0b11111110; // shift left
    I2C_write(address & 0b11111110);     //7 bit address + Write
    I2C_write(data);     //Write data
    I2C_stop();          //Stop condition
    return 0;
}

uint8_t I2C_read_query(uint8_t address, uint8_t *data, uint8_t number_of_bytes){
    I2C_start();         //Start condition
    address = (address << 1)&0b11111110; // shift left
    I2C_write(address | 0b00000001);     //7 bit address + Read
    for(int i=0; i<number_of_bytes; i++){
        data[i] = I2C_read(); // read 
    }
    I2C_stop();          //Stop condition
    return 0;
}

uint8_t I2C_SHT4x_read(float *t_degC, float *rh_pRH){
    I2C_write_query(SHT40_DEVICE_ADDRESS,0xFD);//0x44
    __delay_ms(10);
    uint8_t rx_data[6];
    I2C_read_query(SHT40_DEVICE_ADDRESS, rx_data, 6);
    uint16_t raw_temp = (rx_data[0]<<8) + rx_data[1];
    //uint8_t checksum_t = rx_data[2];
    uint16_t raw_rh = (rx_data[3]<<8) + rx_data[4];
    //uint8_t checksum_rh = rx_data[5];
    *t_degC = -45.0 + (175.0 * (raw_temp/65535.0));
    *rh_pRH = -6.0 + (125.0 * (raw_rh/65535.0));
    if (*rh_pRH > 100){
        *rh_pRH = 100;
    }
    if (*rh_pRH < 0){
        *rh_pRH = 0;
    }
    return 0;
}

uint8_t I2C_SHT4x_read_raw(uint16_t *raw_temp, uint16_t *raw_rh){
    I2C_write_query(SHT40_DEVICE_ADDRESS,0xFD);//0x44
    __delay_ms(10);
    uint8_t rx_data[6];
    I2C_read_query(SHT40_DEVICE_ADDRESS, rx_data, 6);
    *raw_temp = (rx_data[0]<<8) + rx_data[1];
    //uint8_t checksum_t = rx_data[2];
    *raw_rh = (rx_data[3]<<8) + rx_data[4];
    //uint8_t checksum_rh = rx_data[5];
    return 0;
}

void I2C_MCP23008_write(uint8_t address, uint8_t reg_addr, uint8_t data){
    I2C_start();         //Start condition
    address = (address << 1)&0b11111110; // shift left
    I2C_write(address & 0b11111110);     //7 bit address + Write
    I2C_write(reg_addr);     //Write data
    I2C_write(data);
    I2C_stop();          //Stop condition
}

void I2C_MCP23008_read(void){
    uint8_t rx_data[11];
    I2C_read_query(0x27, rx_data, 11);
}