#pragma once

#include <Arduino.h>


#define WIO_E5_RX_BUFFER_SIZE          (512) // NEVER less than 64 bytes
#define WIO_E5_DBG_LINE_MAX_SIZE       (128) // as low as you want if no interesed in debug messages


namespace BlackDevice {

    class Wio_E5 {
        public:

            typedef enum { Band_EU868, Band_US915, } Band_t;

            typedef struct {
                int reset_pin;
                Band_t band;

                const char* appEui;
                const char* devEui;
                const char* appKey;

            } Config_t;

            static const Wio_E5::Config_t Default_Cfg;

            

            Wio_E5(const Config_t* config, HardwareSerial* serial_wio);
            Wio_E5(const Config_t* config, HardwareSerial* serial_wio, Stream* serial_monitor);


            /**
             * @brief Starts the module
             * 
             * @return int 
             */
            int Begin();


            /**
             * @brief Executes a hard reset of the modem
             * 
             */
            void Reset();


            /**
             * @brief Sets the Data Rate
             * 
             * @param dr 
             * @return true 
             * @return false 
             */
            bool Set_Data_Rate(uint8_t dr);


            /**
             * @brief Sets the port
             * 
             * @param port 
             * @return true 
             * @return false 
             */
            bool Set_Port(uint8_t port);


            /**
             * @brief Sends and receive data
             * 
             * @param data 
             * @param data_len 
             * @param rx_data 
             * @param rx_data_len 
             * @param rx_data_rcv 
             * @param rx_port 
             * @return int 
             */
            int Send_Data(uint8_t* data, uint16_t data_len, uint8_t* rx_data = NULL, int rx_data_len = 0, int* rx_data_rcv = NULL, int* rx_port  = NULL);


            /**
             * @brief Returns the last RSSI obtained
             * 
             * @return int 
             */
            int RSSI();

            /**
             * @brief Obtains the At version of the module
             * 
             * @return char* 
             */
            char* At_Version();


            /**
             * @brief Joins to the network
             * 
             * @return true 
             * @return false 
             */
            bool Join();


            /**
             * @brief Indicates if device is joined to the network
             * 
             * @return true 
             * @return false 
             */
            bool Is_Joined();

        private:
            const Config_t* config;
            HardwareSerial* ser_wio;
            Stream* ser_monitor;
            bool is_joined;
            int rssi;
            char fw_version[16];
            char at_buffer[WIO_E5_RX_BUFFER_SIZE];

            int send_at_check_res(const char *p_ack, uint16_t timeout_ms, const char *p_cmd);
            int send_at_command(char* recv_buffer, uint16_t recv_buffer_size, const char *expected_res, uint32_t timeout_ms, const char *p_cmd);
            void get_fw_version();
            void print_dbg(const char* txt, ...);
    };

};

