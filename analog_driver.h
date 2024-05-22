#define BATT_PIN 0
#define VCC_LEVEL 3300.0
#define BATT_MAX_VOLTAGE_MV 6000.0

void Analog_setup(void);
uint16_t Analog_read(void);

void Analog_setup(void){
    /*Settings valid for PIC16F18015*/
    TRISA |= (1<<BATT_PIN);         // Set as input
    ANSELA |= (1<<BATT_PIN);		// Set as analog
    ADCON0bits.CHS = BATT_PIN;       // Select channel : RA0
    ADCON1bits.PREF = 0x0;          // Set positive voltage reference to VDD (default)
    ADCON1bits.CS = 0x0;            // Use ADC internal clock; Fosc/2
    ADCON1bits.FM = 1;              // result right justified
}

uint16_t Analog_read_raw(void){
    ADCON0bits.ON = 1;              // Activate ACD Module
    ADCON0bits.GO = 1;              // Start conversion
    while(ADCON0bits.GO);           // Wait for conversion end
    
    return ADRES;                   // Return result
}

uint16_t Analog_read_voltage(void){
    uint16_t a = Analog_read_raw();
    uint16_t r = a*(VCC_LEVEL/1023.0);  
    return r;                       // Return result (mV)
}

uint8_t Analog_read_percent(void){
    uint16_t a = Analog_read_raw();
    uint16_t voltage = a*(VCC_LEVEL/1023.0);
    
    uint16_t batt_voltage = voltage*2;
    
    uint8_t percent = batt_voltage*(100.0/BATT_MAX_VOLTAGE_MV);  
    return percent;                       // Return result (%)
}