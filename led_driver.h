#define LED_PIN 0

void LED_show_status(uint8_t cycles);
void LED_on();
void LED_off();
void LED_toggle();

void LED_init(){
    TRISA &= !(1<<LED_PIN);		// Set outputs
}

void LED_show_status(uint8_t cycles, uint16_t period_ms){
    for(uint8_t i=0; i<cycles; i++){
        PORTA ^= (1<<LED_PIN);		// toggle LED
        __delay_ms(period_ms >> 1); // delay divided by 2 (which is equivalent to a right shift)
    }
}

void LED_on(){
    PORTA |= (1<<LED_PIN);		// Set LED to 1
}

void LED_off(){
    PORTA &= !(1<<LED_PIN);		// Set LED to 0
}

void LED_toggle() {
    PORTA ^= (1<<LED_PIN);      // Toggle LED
}