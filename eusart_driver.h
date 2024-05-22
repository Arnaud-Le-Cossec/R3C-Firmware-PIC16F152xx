void EUSART_setup(void);
void EUSART_write(uint8_t txData);
void EUSART_print(const char* string);
void EUSART_print_dec(uint8_t number);
void EUSART_print_hex(uint8_t number);
void EUSART_clear_buffer(uint8_t *buffer, uint8_t size);

void EUSART_setup(void){
    /* 16-bit Baud Rate Generator is used */
    BAUD1CONbits.BRG16 = 1;
    /* Asynchronous mode */
    TX1STAbits.SYNC = 0;
    /* Transmit Enable */
    TX1STAbits.TXEN = 1;
    /* High Baud Rate Select */
    TX1STAbits.BRGH = 1;
    /* Continuous receive*/
    RC1STAbits.CREN = 1;
    /* Serial Port Enable */
    RC1STAbits.SPEN = 1;
    /* Baud rate 9600 */
    SP1BRGL = 25;
    SP1BRGH = 0;
    /* RA5 is TX1 */
    RA5PPS = 0x05;
    /* RA4 is RX1*/
    RX1PPS = 0x4;
    /* Configure RA5 as output. */
    TRISAbits.TRISA5 = 0;
    /* Configure RA4 as input. */
    TRISAbits.TRISA4 = 1;
}

void EUSART_write(uint8_t txData){
    while(PIR1bits.TX1IF == 0){}
    TX1REG = txData;
}

uint8_t EUSART_read_wait(void){
    while(PIR1bits.RC1IF==0); // wait
    return RC1REG;
}

void EUSART_print(const char* string){
    uint8_t c=0;
    while(string[c]!=0){
        EUSART_write(string[c]);
        c++;
    }
}

void EUSART_print_dec(uint8_t number){
    uint8_t c = (number/100);
    uint8_t d = (number/10)%10;
    uint8_t u = (number)%10;
    EUSART_write(c+48);
    EUSART_write(d+48);
    EUSART_write(u+48);
}

void EUSART_print_hex(uint8_t number){
    const char ref[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    uint8_t a = (number >> 4)&0b00001111;
    EUSART_write(ref[a]);
    a = number & 0b00001111;
    EUSART_write(ref[a]);
}

void EUSART_clear_buffer(uint8_t *buffer, uint8_t size){
    for(uint8_t i=0; i<size; i++){
        buffer[i] = 0;
    }
}

