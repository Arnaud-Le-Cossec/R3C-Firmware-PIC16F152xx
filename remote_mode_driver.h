#define SCL_PIN RA1
#define SDA_PIN RA2

int remote_check(void){
    TRISAbits.TRISA1 = 1; // Set SCL as input
    TRISAbits.TRISA2 = 0; // Set SDA as output
    
    /*Check if SCL_PIN is low*/
    if (PORTAbits.SCL_PIN == 0){
        /*Acknoledge by pulling SDA low*/
        PORTAbits.SDA_PIN = 0;
        while(PORTAbits.SCL_PIN == 0){
            /*End Acknoledgement*/
            PORTAbits.SDA_PIN = 1;
        }
        return 1;
    }
    /*Not in remote mode, return 0*/
    return 0;
}