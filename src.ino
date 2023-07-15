// RFID define
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 5
#define SS_PIN 6
MFRC522 mfrc522(SS_PIN, RST_PIN);   // RFID(MFRC522)
byte correctCardUID[] = {0x23, 0x1F, 0x11, 0x15};   // Define the correct card UID
byte wrongCardUID[] = {0x03, 0x3B, 0x37, 0x16};   // Define the correct card UID
/////////////////////////////////////////////////////////////////////////////


// PWM define 
#define F_CLK 16000000UL //main clock frequency
#define F_PWM 10000 //PWM frequency (in Hz)
#define TIMER_TOP (unsigned long)(F_CLK/(2*F_PWM)-1)
#define PIN1_PWM 9
#define PIN2_PWM 10
#define PIN_LED  13
#define PIN_CURRENT A0 // replace with the pin connected to the sensor output

// Buzzer define
#define BUZZER_PIN 7 // Digital pin connected to the buzzer

/////////////////////////////////////////////////////////////////////////////


// global variables 
int dutyStart = 420; // between O to 1000 (O for 0% and 1000 for 100.0% duty cycle)
int dutyStop = 580;
int32_t ocr;
float MAX_CURRENT = 1;
int oc_count=0;
int count=0;
float current=0;
/////////////////////////////////////////////////////////////////////////////


// setup
void setup() 
  {
    // initialize
    Serial.begin(9600);   // Initialize serial communication
    SPI.begin();  // Initialize SPI bus
    mfrc522.PCD_Init();  // Initialize MFRC522
    Serial.println("----Tap to open----");

    ///////////////// Timerl config 1//////////////////
    TCCR1A=_BV(COM1A1) | _BV(COM1A0)  | _BV(COM1B1);   //OCIA and OCIB
    TCCR1B=_BV(WGM13) | _BV(CS10);     // Phase and Frequency Correct PWM, N=1
    ICR1=TIMER_TOP;

    pinMode(BUZZER_PIN, OUTPUT);
  }
/////////////////////////////////////////////////////////////////////////////


// Functions
// A user defined function to update PWM registers (Bipolar)
// duty is duty cycle (O for 0% and 1000 for 100.0% duty cycle)
void update_pwm_bipolar(unsigned int d)
  {
    ocr=d*TIMER_TOP/1000;
    OCR1A = ocr;
    OCR1B = ocr;
  }

// stop PWM function
void stopPWM() 
  {
    TCCR1A = 0;  // Clear TCCR1A register
    TCCR1B = 0;  // Clear TCCR1B register
    pinMode(PIN1_PWM, INPUT);  // Set PWM pin as input
    pinMode(PIN2_PWM, INPUT);  // Set PWM pin as input
    Serial.println("/// PWM stopped ///");
  }

// read current function
float read_current(void)
{
  int adc = analogRead(A0);
  float voltage = adc * 5 / 1023.0;
  float current = (voltage - 2.5032) / 0.3373;
  return current;
}

// Buzz the buzzer for 0.5 seconds
void buzzBuzzer()
{
  tone(BUZZER_PIN, 1000); // Start buzzing at 1000Hz
  delay(1000); // Buzz for 0.5 seconds
  noTone(BUZZER_PIN); // Stop buzzing
}
/////////////////////////////////////////////////////////////////////////////


//loop
void loop() {
  while(2){
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
  {
    Serial.println("enter");
    buzzBuzzer();
    // Check if the card UID matches the correct UID
    if (memcmp(mfrc522.uid.uidByte, correctCardUID, mfrc522.uid.size) == 0)
    {
      // link PWM 
      pinMode(PIN_LED, OUTPUT) ;
      digitalWrite(PIN_LED, HIGH) ;
      pinMode(PIN1_PWM,OUTPUT) ;
      pinMode(PIN2_PWM,OUTPUT);

      // Activate PWM, duty = start
        Serial.println("/// forward ///");
        update_pwm_bipolar(dutyStart);
            count=7000;
            oc_count=0;
            while(count--)
              if (read_current()>MAX_CURRENT)
              {
                if (oc_count++>2000)
                {
                  Serial.println("/// overcurrent ///");
                  stopPWM();
                  return loop();
                }
              }
        //delay(3000);

      // pause PWM
        Serial.println("/// paused ///");
        update_pwm_bipolar(500);
            count=30000;
            oc_count=0;
            while(count--)
              if (read_current()>MAX_CURRENT)
              {
                if (oc_count++>2000)
                {
                  Serial.println("/// overcurrent ///");
                  stopPWM();
                  return loop();
                }
              }        
        buzzBuzzer();
        //delay(3000);

      // Change duty cycle to 750
        Serial.println("/// reverse ///");
        update_pwm_bipolar(dutyStop); 
            count=8300;
            oc_count=0;
            while(count--)
              if (read_current()>MAX_CURRENT)
              {
                if (oc_count++>2000)
                {
                  Serial.println("/// overcurrent ///");
                  stopPWM();
                  return loop();
                }
              }            
        //delay(3000);

      //stop generating PWM
        Serial.println("/// stopped ///");
        stopPWM();
        Serial.println("/// demo ///");
        return loop();
  }

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    return loop();
  }
  
  }
  Serial.println("/// demo ///");
 return loop();
}
void loop();
