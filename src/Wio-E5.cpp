/**
 * @file Wio-E5.cpp
 * @author BlackDevice (info@blackdevice.es)
 * @brief Library to manage Wio-E5 module. This SoC uses, by default, an AT command interface.
 * This library implements the more common AT commands to send and receive data frames over LoRaWan protocol
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Wio-E5.h"

static const char to_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };


/**
 * @brief Converts HEX char to 4-bit decimal value. Returns 0x80 in case of error
 * 
 * @param data 
 * @return uint8_t 
 */
static uint8_t parse_hex(char* data) {
    if (data == NULL) return 0x80;

    if (*data >= '0' && *data <= '9') return *data - '0';
    if (*data >= 'a' && *data <= 'f') return *data - 'a' + 10;
    if (*data >= 'A' && *data <= 'F') return *data - 'A' + 10;

    return 0x80;
}



namespace BlackDevice {

    const Wio_E5::Config_t Wio_E5::Default_Cfg = {
        .reset_pin = 21,
        .band = Wio_E5::Band_EU868,
        .appEui = NULL,
        .devEui = NULL,
        .appKey = NULL,
    };


    Wio_E5::Wio_E5(const Config_t* config, HardwareSerial* serial_wio) : Wio_E5(config, serial_wio, NULL) { }

    Wio_E5::Wio_E5(const Config_t* config, HardwareSerial* serial_wio, Stream* serial_monitor) {
        this->config = config;
        ser_wio = serial_wio;
        ser_monitor = serial_monitor;

        is_joined = false;
        rssi = 0;
        fw_version[0] = 0;
    }


    /**
     * @brief Starts the module
     * 
     * @return int 
     */
    int Wio_E5::Begin() {
        is_joined = false;
        rssi = 0;
        fw_version[0] = 0;

        if (!ser_wio) {
            print_dbg("No serial port defined for Wio-E5 module :-(\r\n");
            return 0;
        }

        print_dbg("Library begins...\r\n");

        ser_wio->begin(9600, SERIAL_8N1);

        Reset();

        int cnt = 10;
        while (--cnt) {
            if (send_at_check_res("+AT: OK", 500, "AT\r\n")) break;
            delay(100);
        }

        if (cnt == 0) {
            print_dbg("No Wio-E5 module found :-(\r\n");
            return 0;
        }

        get_fw_version();

        // set the band
        if (this->config->band == Band_EU868) send_at_check_res("+DR: EU868", 1000, "AT+DR=EU868\r\n");
        if (this->config->band == Band_US915) send_at_check_res("+DR: US915", 1000, "AT+DR=US915\r\n");

        // set the keys
        snprintf(at_buffer, sizeof(at_buffer), "AT+ID=DevEUI,\"%s\"\r\n", this->config->devEui);
        send_at_check_res("+ID: DevEui", 1000, at_buffer);

        snprintf(at_buffer, sizeof(at_buffer), "AT+ID=AppEUI,\"%s\"\r\n", this->config->appEui);
        send_at_check_res("+ID: AppEui", 1000, at_buffer);

        snprintf(at_buffer, sizeof(at_buffer), "AT+KEY=APPKEY,\"%s\"\r\n", this->config->appKey);
        send_at_check_res("+KEY: APPKEY", 1000, at_buffer);

        // Set LWOTAA mode
        send_at_check_res("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");

        // Set LoRa Class A
        send_at_check_res("+CLASS: A", 1000, "AT+CLASS=A\r\n");

        // joins to the 
        Join();

        return 1;
    }


    /**
     * @brief Executes a hard reset of the modem
     * 
     */
    void Wio_E5::Reset() {
        // reset pin as output
        pinMode(config->reset_pin, OUTPUT);
        delay(1);

        digitalWrite(config->reset_pin, LOW);
        delay(500);
        digitalWrite(config->reset_pin, HIGH);
        delay(500);
        digitalWrite(config->reset_pin, LOW);
        delay(500);
    }


    /**
     * @brief Joins to the network
     * 
     * @return true 
     * @return false 
     */
    bool Wio_E5::Join() {
        is_joined = send_at_check_res("+JOIN: Network joined", 12000, "AT+JOIN\r\n");

        return is_joined;
    }


    /**
     * @brief Sets the Data Rate
     * 
     * @param dr 
     * @return true 
     * @return false 
     */
    bool Wio_E5::Set_Data_Rate(uint8_t dr) {
        if (dr > 15) return false;

        char cmd[16];
        sprintf(cmd, "AT+DR=%d\r\n", dr);
        return (send_at_check_res("+DR: ", 1000, cmd) > 0);
    }


    /**
     * @brief Sets the port
     * 
     * @param port 
     * @return true 
     * @return false 
     */
    bool Wio_E5::Set_Port(uint8_t port) {
        if (!port) return false;

        char cmd[32];
        sprintf(cmd, "AT+PORT=%d\r\n", port);
        return (send_at_check_res("+PORT: ", 1000, cmd) > 0);
    }


    /**
     * @brief Indicates if device is joined to the network
     * 
     * @return true 
     * @return false 
     */
    bool Wio_E5::Is_Joined() { return is_joined; }


    /**
     * @brief Returns the last RSSI obtained
     * 
     * @return int 
     */
    int Wio_E5::RSSI() { return rssi; }


    /**
     * @brief Obtains the At version of the module
     * 
     * @return char* 
     */
    char* Wio_E5::At_Version() { return fw_version; }



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
    int Wio_E5::Send_Data(uint8_t* data, uint16_t data_len, uint8_t* rx_data /*= NULL*/, int rx_data_len /*= 0*/, int* rx_data_rcv /*= NULL*/, int* rx_port /*= NULL*/) {
        if ((data_len*2 + 16) > sizeof(at_buffer)) {
            print_dbg("send_data. Buffer overflow. Available data size is [%d] bytes]\r\n", (sizeof(at_buffer) - 16)/2);
            return 0;
        }

        if (!Is_Joined()) if (!Join()) {
            print_dbg("send_data. Not joined to the network :-(\r\n");
            return 0;
        }

        // create AT command
         strcpy(at_buffer, "AT+CMSGHEX=\"");
        // converts binary data to hex data
        char* ptr = &at_buffer[strlen(at_buffer)];
        for (uint16_t i = 0; i < data_len; i++) {
            *(ptr++) = to_hex[data[i] >> 4];
            *(ptr++) = to_hex[data[i] & 0x0f];
        }
        strcpy(ptr, "\"\r\n");

        // launches the AT command
        if (!send_at_command(at_buffer, sizeof(at_buffer), "Done", 8000, at_buffer)) return 0;


        // let's extract RSSI from response
        int rssi = 0;
        ptr = strstr(at_buffer, "RSSI");
        if (ptr) if (sscanf(ptr, "RSSI %d,", &rssi) == 1) this->rssi = rssi;

        // let's extract RX data from response
        ptr = at_buffer;

        if (rx_port != NULL) {
            *rx_port = 0;
            ptr = strstr(at_buffer, "PORT: ");
            if (ptr) sscanf(ptr, "PORT: %d;", rx_port);
        }

        if (rx_data != NULL && rx_data_len > 0 && rx_data_rcv != NULL) {
            *rx_data_rcv = 0;

            ptr = strstr(at_buffer, "RX: \"");

            if (ptr) {
                // found response
                ptr += 5; // skips string [RX: "]
                while (1) {
                    uint8_t r = parse_hex(ptr++);
                    if (r > 0x0f) break;
                    rx_data[*rx_data_rcv] = (r << 4);

                    r = parse_hex(ptr++);
                    if (r > 0x0f) break;
                    rx_data[*rx_data_rcv] |= r;
                    (*rx_data_rcv)++;

                    if (*rx_data_rcv >= rx_data_len) break;
                }
            }
        }
        
        return 1;
    }


    void Wio_E5::get_fw_version() {
        char response[32];
        if (!send_at_command(response, sizeof(response), "+VER: ", 5000, "AT+VER\r\n")) return;

        char* ptr = response;
        while (*ptr < '0' || *ptr > '9') if (*ptr == 0) return; else ptr++;
        
        char* ptr_dest = fw_version;
        char* ptr_dest_end = ptr_dest + sizeof(fw_version) - 1;

        while (1) {
            if (*ptr == 0 || *ptr == '\r' || *ptr == '\n') break;

            *(ptr_dest++) = *(ptr++);

            if (ptr_dest >= ptr_dest_end) break;
        }

        *ptr_dest = 0; // end of string
    }



    int Wio_E5::send_at_check_res(const char* p_ack, uint16_t timeout_ms, const char *p_cmd) {
        if (p_cmd == NULL) return 0;

         return send_at_command(at_buffer, sizeof(at_buffer), p_ack, timeout_ms, p_cmd);
    }



    int Wio_E5::send_at_command(char* recv_buffer, uint16_t recv_buffer_size, const char *expected_res, uint32_t timeout_ms, const char *p_cmd) {

        // flush the serial rx buffer
        while (ser_wio->available() > 0) ser_wio->read(); 

        ser_wio->print(p_cmd); // sends the AT command
        print_dbg("> %s", p_cmd);

        if (expected_res == NULL) return 1;
        if (*expected_res == 0) return 1;
        if (recv_buffer_size == 0) return 0;
        if (recv_buffer == NULL) return 0;
        
        // wait for the expected response
        char rx_frame[32];
        uint16_t recv_buf_len = 0;

        recv_buffer[0] = 0;
        timeout_ms += millis();

        while (1) {
            // read a frame from Serial port
            uint16_t rx_frame_len = 0;
            uint32_t rx_frame_millis = millis() + 20;
            while (rx_frame_millis > millis()) {
                if (ser_wio->available()) {
                    rx_frame[rx_frame_len++] = ser_wio->read();
                    if (rx_frame_len >= (uint16_t)(sizeof(rx_frame) - 1)) break;
                    rx_frame_millis = millis() + 50;
                } else delay(2);
            }

            if (rx_frame_len > 0) {
                // data received in Serial
                rx_frame[rx_frame_len] = 0;
                print_dbg(rx_frame);
                    
                if ((recv_buf_len + rx_frame_len) > (recv_buffer_size - 1)) {
                    print_dbg(" !! Buffer overflow :-(\r\n");
                    return 0; // buffer overflow
                }

                memcpy(&recv_buffer[recv_buf_len], rx_frame, rx_frame_len);
                recv_buf_len += rx_frame_len;
                recv_buffer[recv_buf_len] = 0;

                if (strstr(recv_buffer, expected_res) != NULL) return 1; // command executed succesfully
            }

            if (millis() > timeout_ms) break;
        }

        // timeout
        print_dbg(" !! No response\r\n");
        return 0;
    }


    void Wio_E5::print_dbg(const char* txt, ...) {
        static char dbg_line[WIO_E5_DBG_LINE_MAX_SIZE];
        if (!ser_monitor) return; // nothing to do
        if (sizeof(dbg_line) < 32) return; // no enough space reserved

        sprintf(dbg_line, "[%lu] Wio-E5: ", millis());
        ser_monitor->print(dbg_line);

        va_list args;
        va_start(args, txt);
        vsnprintf(dbg_line, sizeof(dbg_line), txt, args);
        va_end(args);

        ser_monitor->print(dbg_line);
    }

}