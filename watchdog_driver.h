void WDT_setup(void);
void SLEEP_start(void);

void WDT_setup(void){
    /*Setup watchdog for slowest interval : 256 sec*/
    //0b10010 - 256 s
    //0b10000 - 64 s
    WDTCONbits.PS = 0b10000;
}

void SLEEP_start(void){
    /*Assembly instruction to reset watchdog*/
    asm("CLRWDT");
    /*Assembly instruction to put MCU into sleep*/
    asm("SLEEP");
}