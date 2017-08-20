/* DHT11 temperature and humidity sensor
 * Project tutorial url: http://kookye.com/2015/03/21/dht11-temperature-and-humidity-sensor/
 * Circuit diagram here: http://osoyoo.com/2015/03/21/dht11-temperature-and-humidity-sensor/
 * 
 * Sample code: http://kookye.com/wp-content/uploads/samplecode/dht11.txt
 * Rewritten by https://github.com/phawxby
 */

// include the library code:
#include <LiquidCrystal.h>

int dht11pin = A0;
static const unsigned long MAX_SWITCH_ON_TIME = 300000; // 5 minutes
static const int MAX_HUMIDITY = 50; // Ideal (40-60) percent

#define RELAY1 7
#define DEBUG false

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);

void setup(){
  if (DEBUG)
    Serial.begin(9600);
  else
    lcd.begin(16, 2);
  pinMode(RELAY1, OUTPUT);
  delay(300);
  delay(700);
}

void loop(){

  // We can't return an array from a function, so we pass in an array to modify
  byte dhtData[6];
  // 0 = Error state
  // 1 = Humidity whole
  // 2 = Humidity decimal
  // 3 = Temp whole
  // 4 = Temp decimal
  // 5 = Checksum
  ReadDHT11(dht11pin, dhtData);

  switch (dhtData[0]){
    case 0:
       lcd.setCursor(0, 0);
       Print("Humidity = ");
       Print(dhtData[1]);
       Print(".");
       Print(dhtData[2]);
       Print("%  ");
       lcd.setCursor(0, 1);
       Print("Temp = ");
       Print(dhtData[3]);
       Print(".");
       Print(dhtData[4]);
       PrintLn("C");

       if (dhtData[1] > MAX_HUMIDITY)
          SwitchOn();
          
    break;
    case 1:
      PrintError("Error 1: DHT start condition 1 not met.");
      break;
    case 2:
      PrintError("Error 2: DHT start condition 2 not met.");
      break;
    case 3:
      PrintError("Error 3: DHT checksum error.");
      break;
    default:
      PrintError("Error: Unrecognized code encountered.");
      break;
  }

  // Read the temperature every 5 seconds
  delay(2000);
}

void ReadDHT11(int pin, byte* output)
{
  // Init DHT
  pinMode(pin,OUTPUT);
  digitalWrite(pin,HIGH);
  delayMicroseconds(20);
  // Trigger data output
  digitalWrite(pin,LOW);
  delay(20);
  digitalWrite(pin,HIGH);
  delayMicroseconds(40);

  // DHT in var
  byte dht_in;

  // Start reading
  pinMode(pin, INPUT);
  dht_in = digitalRead(pin);

  // Nothing should be returned to begin with
  if(dht_in){
    // Error mode 1
    output[0] = 1;
    return;
  }

  // Wait 80 microseconds and recheck
  delayMicroseconds(80);
  dht_in = digitalRead(pin);

  // This time something should be there
  if(!dht_in){
    // Error mode 2
    output[0] = 2;
    return;
  }

  // Wait 80 microseconds then we should get data
  delayMicroseconds(80);

  // Loop 5 times
  // 1 = Humidity whole
  // 2 = Humidity decimal
  // 3 = Temp whole
  // 4 = Temp decimal
  // 5 = Checksum
  byte i = 1;
  for (i=1; i<6; i++) {
    output[i] = ReadDHT11Pin(pin);
  }

  byte dht_check_sum = output[1] + output[2] + output[3] + output[4];
  if(output[5]!= dht_check_sum) {
    // Checksum doesn't match, something went wrong
    output[0] = 3;
  } else {
    // Response passes checksum
    output[0] = 0;
  }

  // Reset the output to high
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

byte ReadDHT11Pin(int pin){
  byte i = 0;
  byte result=0;

  // We need to read one byte of data off the data pin, read this bit by bit, literally, by looping 8 times
  for(i=0; i<8; i++) {
    // Stall the loop while the pin is low, wait for it to go high indicating incoming bit
    while(digitalRead(pin)==LOW);
    // Pin goes high briefly to notify of incoming bit. Wait 30 microseconds to allow the pin to drop back to low if necessary
    delayMicroseconds(30);
    // If the pin goes high then we have data at this position in the byte
    if (digitalRead(pin)==HIGH) {
      // Set the bit
      result |=(1<<(7-i));
    }
    // Stall the loop until the pin drops back to low indicating either no data at this bit or the end of the high bit
    while (digitalRead(pin)==HIGH);
  }
  
  return result;
}

void SwitchOn() {
  digitalWrite(RELAY1, HIGH);
  PrintLn("Switched on.");
  delay(MAX_SWITCH_ON_TIME);
  SwitchOff();
}

void SwitchOff() {
  digitalWrite(RELAY1, LOW);
  PrintLn("Switched off.");
  delay(20000);
}

void PrintLn(const char* input) {
  if (DEBUG) {
    Serial.println(input);
    return;
  }
  lcd.print(input);
}

void Print(const char* input) {
  if (DEBUG) {
    Serial.print(input);
    return;
  }
  lcd.print(input);
}

void Print(byte input) {
  if (DEBUG) {
    Serial.print(input, DEC);
    return;
  }
  lcd.print(input, DEC);
}

void PrintError(const char* error) {
  if (!DEBUG)
    return;
  Serial.println(error);
  return;
}

