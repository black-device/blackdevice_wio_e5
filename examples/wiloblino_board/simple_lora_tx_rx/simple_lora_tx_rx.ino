/**
 * @file simple_lora_tx_rx.ino
 * @author BlackDevice (info@blackdevice.es)
 * @brief Simple example to send and receive data over a LoRaWan infraestructure.
 * This code was developed to work properly in a stand-alone Wiloblino board
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include "Wio-E5.h"


/**
 * @brief Wiloblino pins
 * 
 */
#define WILOBLINO_LED_PIN                   (2)
#define WILOBLINO_WIO_E5_RESET_PIN          (21)


/**
 * @brief Lora keys. Change at discretion
 * 
 */
const char *appEui = "0123456789abcdef";
const char *devEui = "22446688aabbccdd";
const char *appKey = "9876543210abcdef9876543210abcde0";


/**
 * @brief port used for transmision
 * 
 */
const int tx_port = 99;




HardwareSerial* serial_monitor;
HardwareSerial* serial_wio_e5;
BlackDevice::Wio_E5 *lora;
BlackDevice::Wio_E5::Config_t lora_cfg;

void setup() {
    
    // setup serial ports. Obtains the reference of the serial ports
    serial_wio_e5 = &Serial; // port used for LoRa Module
    serial_monitor = &Serial2; // port used for debug

    serial_monitor->begin(115200, SERIAL_8N1);
    delay(10);

    serial_monitor->println("\r\n\r\n\r\n\r\n...\r\n..\r\n.\r\nSimple Wiloblino-LoRa Example ..");
    delay(2000);

    // LoRa module configuration
    lora_cfg = BlackDevice::Wio_E5::Default_Cfg;
    
    lora_cfg.reset_pin = WILOBLINO_WIO_E5_RESET_PIN;
    lora_cfg.band = BlackDevice::Wio_E5::Band_EU868;
    lora_cfg.appEui = appEui;
    lora_cfg.devEui = devEui;
    lora_cfg.appKey = appKey;

    // create the object
    lora = new BlackDevice::Wio_E5(&lora_cfg, serial_wio_e5, serial_monitor);

    // launches the libray and joins to the gateway
    lora->Begin();

    serial_monitor->print("$$$> AT Module version: ");
    serial_monitor->println(lora->At_Version());

}

void loop() {
    
    // blinks the led once per second
    pinMode(WILOBLINO_LED_PIN, OUTPUT);
    digitalWrite(WILOBLINO_LED_PIN, LOW);
    delay(975);
    digitalWrite(WILOBLINO_LED_PIN, HIGH);
    delay(25);

    static int cnt = 50;
    if (cnt++ % 60) return;

    // Try to publish each ~60 seconds
    if (!lora->Is_Joined()) {
        if (!lora->Join()) {
            serial_monitor->println("$$$> Not joined :-(");
            return;
        }
    }

    static uint8_t tx_data[64];
    static uint8_t rx_data[256];

    // string to publish
    snprintf((char*)tx_data, sizeof(tx_data), "{\"counter_value\": %d}", cnt);

    
    int rx_recv = 0;
    int rx_port  = 0;

    // setup tx port
    lora->Set_Port(tx_port);

    if (!lora->Send_Data(tx_data, strlen((char*)tx_data), rx_data, sizeof(rx_data), &rx_recv, &rx_port)) {
        serial_monitor->println("$$$> Tx failed :-(");
        return;
    }

    serial_monitor->print("$$$> RSSI: ");
    serial_monitor->println(lora->RSSI());


    if (rx_recv > 0) {
        serial_monitor->print("$$$> Data received. [");
        serial_monitor->print(rx_recv);
        serial_monitor->print("] bytes in port [");
        serial_monitor->print(rx_port);
        serial_monitor->print("]: ");
        for (int i = 0; i < rx_recv; i++) serial_monitor->print(rx_data[i], HEX);
        serial_monitor->println();
    }

}
