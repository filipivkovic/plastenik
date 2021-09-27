#include <RH_ASK.h> // RadioHead javna biblioteka za RF komunikaciju 
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> //standardni SPI protokol Arduino biblioteka
#endif
#include <Sensirion.h> //biblioteka za digitalni senzor vlaznosti vazduha i temperature

const uint8_t dataPin  =  10; //pinovi na arduinu za SPI komunikaciju 
const uint8_t clockPin =  11; //pinovi na arduinu za SPI komunikaciju 
float temperature;
float humidity;
float dewpoint;

Sensirion tempSensor = Sensirion(dataPin, clockPin);
//struktura za protokol RF komunikacije
struct dataStruct {
  byte id;
  byte komanda;
  byte CRC;
  float result;
} myData;


uint8_t tx_buf[sizeof(myData)] = {0};

//podesavanje pinova za RF protokol, pin 2 - RX za primanje, pin 6 - TX za slanje)
RH_ASK driver(2000, 2, 6, 0);

void setup() {
  pinMode(8, OUTPUT); //inicijalizacija pin 8 kao izlazni pin
  pinMode(9, OUTPUT); //inicijalizacija pin 9 kao izlazni pin
  digitalWrite(8, HIGH); //inicijalizacija pin 8 na stanje logicke jedinice
  digitalWrite(9, HIGH); //inicijalizacija pin 9 na stanje logicke jedinice
  myData.CRC = 0;
  driver.init();
}

void loop() {

  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);

  if (driver.recv(buf, &buflen)) //ceka se primanje signala 
  {
    memcpy(&myData, buf, sizeof(myData));
    if (myData.id == 01) //ako signal odgovara protokolu, nastavi
    {
      switch (myData.komanda)
      {
        case 01: //ako je komanda 01, procitaj vrednost temperature vazduha
          tempSensor.measure(&temperature, &humidity, &dewpoint);
          myData.result = temperature;
          memcpy(tx_buf, &myData, sizeof(myData) );
          driver.send((uint8_t *)tx_buf, sizeof(myData));
          driver.waitPacketSent();
          delay(100);
          break;
        case 02: //ako je komanda 02, procitaj vrednost vlaznosti vazduha
          tempSensor.measure(&temperature, &humidity, &dewpoint);
          myData.result = humidity;
          memcpy(tx_buf, &myData, sizeof(myData) );
          driver.send((uint8_t *)tx_buf, sizeof(myData));
          driver.waitPacketSent();
          delay(100);
          break;
        case 03: //ako je komanda 03, procitaj vrednost tacke na kojoj se vrsi kondenzacija vode
          tempSensor.measure(&temperature, &humidity, &dewpoint);
          myData.result = dewpoint;
          memcpy(tx_buf, &myData, sizeof(myData) );
          driver.send((uint8_t *)tx_buf, sizeof(myData));
          driver.waitPacketSent();
          delay(100);
          break;
        case 04: //ako je komanda 04, procitaj vrednost na analognom ulazu koji predstavlja jacinu osvetljenosti u Lux jedinicama
          myData.result = analogRead(A0)/10.24;
          memcpy(tx_buf, &myData, sizeof(myData) );
          driver.send((uint8_t *)tx_buf, sizeof(myData));
          driver.waitPacketSent();
          delay(100);
          break;
          
      }
    }
  }
  delay(2000); // uredjaj ne radi nista, cuva energiju, nalazi se u standby modu dve sekunde
}
