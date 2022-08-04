/**
 * @file simple_lora_tx_rx.ino
 * @author BlackDevice (info@blackdevice.es)
 * @brief Simple example to send and receive data over a LoRaWan infraestructure.
 * This code was developed to work in an Arduino MEGA board with Wiloblino board as hat
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include "Wio-E5.h"


/**
 * @brief pins
 * 
 */
#define MEGA_LED_PIN                        (13)
#define WIO_E5_RESET_PIN                    (8)
#define DBG_SOFTSERIAL_RX                   (2)
#define DBG_SOFTSERIAL_TX                   (3)


/**
 * @brief Lora configuration. Change at discretion
 * 
 */
/* LoRa Keys */
const char *appEui = "0123456789abcdef";
const char *devEui = "22446688aabbccdd";
const char *appKey = "9876543210abcdef9876543210abcde0";

/* Lora Band. Setup properly for your zone */
const BlackDevice::Wio_E5::Band_t default_lora_band = BlackDevice::Wio_E5::Band_EU868;

/* Default port to transmition */
const int defaut_tx_port = 99;
/*-----------------------------------------------*/


HardwareSerial* serial_monitor;
HardwareSerial* serial_wio_e5;
BlackDevice::Wio_E5 *lora;
BlackDevice::Wio_E5::Config_t lora_cfg;

void setup() {
    
    // setup serial ports. Obtains the reference of the serial ports
    serial_wio_e5 = &Serial; // port used for LoRa Module
    serial_monitor = &Serial1; // port used for debug

    serial_monitor->begin(115200);
    delay(10);

    serial_monitor->println("\r\n\r\n\r\n\r\n...\r\n..\r\n.\r\nSimple ArduMEGA-Wiloblino-LoRa Example ..");
    delay(2000);

    // LoRa module configuration
    lora_cfg = BlackDevice::Wio_E5::Default_Cfg;
    
    lora_cfg.reset_pin = WIO_E5_RESET_PIN;
    lora_cfg.band = default_lora_band;
    lora_cfg.appEui = appEui;
    lora_cfg.devEui = devEui;
    lora_cfg.appKey = appKey;

    lora = new BlackDevice::Wio_E5(&lora_cfg, serial_wio_e5, serial_monitor);
    lora->Begin();

    serial_monitor->print("$$$> AT Module version: ");
    serial_monitor->println(lora->At_Version());

}

void loop() {
    
    // blinks the led once per second
    pinMode(MEGA_LED_PIN, OUTPUT);
    digitalWrite(MEGA_LED_PIN, LOW);
    delay(975);
    digitalWrite(MEGA_LED_PIN, HIGH);
    delay(25);

    static int cnt = 50;
    if (cnt++ % 30) return;

    // Try to publish each ~60 seconds
    if (!lora->Is_Joined()) {
        if (!lora->Join()) {
            serial_monitor->println("$$$> Not joined :-(");
            return;
        }
    }
    static uint8_t tx_data[64];
    static uint8_t rx_data[256];
    static int error_cnt = 0;

    // string to publish
    snprintf((char*)tx_data, sizeof(tx_data), "{\"counter_value\": %d}", cnt);

    
    int rx_recv = 0;
    int rx_port  = 0;

    // setup tx port
    lora->Set_Port(defaut_tx_port);

    if (!lora->Send_Data(tx_data, strlen((char*)tx_data), rx_data, sizeof(rx_data), &rx_recv, &rx_port)) {
        serial_monitor->println("$$$> Tx failed :-(");
        if (++error_cnt > 0) {
          serial_monitor->println("$$$> Restarting Wio-E5 module...");
          lora->Begin();
        }
        return;
    }
    error_cnt = 0;

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
