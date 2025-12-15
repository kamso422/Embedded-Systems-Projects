#include <SPI.h>
#include <LiquidCrystal.h>

// Initialize the LCD with the numbers of the interface pins
// (RS, Enable, D4, D5, D6, D7)
// **Adjust these pin numbers based on your actual wiring**
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
uint8_t length = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variable to store the data received from the SPI Master
char receivedData[14];

// A flag to indicate that new data has been received
volatile bool newDataFlag = false;

// --- PIN DEFINITIONS FOR SPI ---
// SCK: Pin 13 (or D13) on Uno/Nano
// MOSI: Pin 11 (or D11) on Uno/Nano
// MISO: Pin 12 (or D12) on Uno/Nano
// SS (Slave Select): Pin 10 (or D10) on Uno/Nano

void setup() {
  Serial.begin(9600);
  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.print("SPI Slave Ready");
  lcd.setCursor(0, 1);
  lcd.print("Waiting for data");

  // Initialize SPI as a Slave
  // 1. Turn on SPI enable, Interrupt enable
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPIE);

  // 2. Set MISO as an OUTPUT (the only output for a slave)
  // MOSI, SCK, and SS are automatically configured as inputs
  pinMode(MISO, OUTPUT);

  // 3. Set the Slave Select (SS) pin as an INPUT with pull-up enabled
  // This is a good practice to prevent the pin from floating
  pinMode(SS, INPUT_PULLUP);

  // 4. Set the initial data to send back to the Master (optional)
  // This will be the first data the Master receives on its MISO line
  SPDR = 0xAA; // Example handshake value

  // 5. Enable global interrupts
  // The SPI_STC_vect will not execute without this
  sei();
}

void loop() {
  // Check if the newDataFlag is set by the interrupt service routine
  if (newDataFlag) {
    // Clear the flag immediately to be ready for the next data
    newDataFlag = false;

    // Clear the display and show the received value
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Received Data:");

    // Display the received byte as an integer (Decimal)
    lcd.setCursor(0, 1);
    lcd.print(receivedData);
    Serial.println(receivedData);
  }

  // The main loop can perform other tasks here while waiting for
  // the SPI interrupt to trigger.
}

// --- SPI Serial Transfer Complete Interrupt Service Routine (ISR) ---
// This function is automatically called every time a complete byte
// has been shifted into the SPDR register.
ISR(SPI_STC_vect) {
  static uint8_t counter = 0;
  static uint8_t i = 0;
  // Read the received data from the SPI Data Register
  if(counter == 0){
    length = SPDR;
    Serial.println(length);
    counter++;
  }else{
       receivedData[i] = SPDR;
        i++;    
        // Set the flag to signal the main loop to update the display
        if(i >= length){
          newDataFlag = true;
        }
    }
  // **IMPORTANT SLAVE BEHAVIOR:**
  // Before the ISR finishes, we should load the next byte we want to send
  // back to the Master during the *next* transfer.
  // In this example, we simply send back the value we just received + 1,
  // confirming the reception to the Master.
  SPDR = receivedData[i] + 1;
}
