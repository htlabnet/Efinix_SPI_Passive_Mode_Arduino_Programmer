//
//
//    Efinix SPI Passive Mode Arduino Programmer
//
//     https://github.com/htlabnet/Efinix_SPI_Passive_Mode_Arduino_Programmer
//
//    Copyright (C) 2023
//      Hideto Kikuchi / PJ (@pcjpnet) - http://pc-jp.net/
//      Tsukuba Science Inc. - http://www.tsukuba-kagaku.co.jp/
//
//

//
// !!!!! Important !!!!!
//
//  Generate "Bitstream.h" using "efinix_constant_converter",
//  located in the software folder.
//

//
//   Seeed Studio XIAO ESP32C3
//
// PIN : FPGA
// D1  : SS (OUTPUT, MCU->FPGA)
// D2  : CDONE (INPUT, FPGA->MCU)
// D3  : CRESET_N (OUTPUT, MCU->FPGA)
// D8  : CCK  (SPI,SCK,OUTPUT, MCU->FPGA)
// D9  :  *   (SPI,MISO,INPUT)
// D10 : CDI0 (SPI,MOSI,OUTPUT)
//
// IDE:   Arduino IDE 2.1.0
// Board: esp32 by Espressif Systems 2.0.9
// Chip:  ESP32-C3 (revision v0.3)
//
// Flash : 878180 Bytes / 66%
// SRAM  : 15884 Bytes / 4%
//


#include <SPI.h>
#include "Bitstream.h"
//#include "src/Bitstream.h"


#define PIN_FPGA_SS D1
#define PIN_FPGA_CDONE D2
#define PIN_FPGA_RESET D3


void setup() {


  delay(5000);


  // Start Configuration
  SPI.begin();
  SPI.setFrequency(10000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  pinMode(PIN_FPGA_SS, OUTPUT);
  pinMode(PIN_FPGA_CDONE, INPUT);
  pinMode(PIN_FPGA_RESET, OUTPUT);
  digitalWrite(PIN_FPGA_RESET, LOW);
  digitalWrite(PIN_FPGA_SS, HIGH);
  delayMicroseconds(10);

  digitalWrite(PIN_FPGA_SS, LOW);
  delayMicroseconds(10);

  digitalWrite(PIN_FPGA_RESET, HIGH);
  delayMicroseconds(10);

  // Write Bitstream Data
  for (uint32_t i = 0; i < bitstream_byte_size; i++) {
    SPI.write(bitstream_data[i]);
  }

  // Write Extra Clock
  for (uint8_t i = 0; i < 125; i++) {
    SPI.write(0x00);
  }

  delayMicroseconds(10);
  digitalWrite(PIN_FPGA_SS, HIGH);

  // Config Done Check
  while (!digitalRead(PIN_FPGA_CDONE)) {
    delay(10);
  }
  // Configration Done !!!



}


void loop() {

}

