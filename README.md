# Wio-E5 Arduino library

Wio-E5 is Wireless module for long range applications. you can get it in [seeed studio](https://www.seeedstudio.com/LoRa-E5-Wireless-Module-p-4745.html)

This radio with it's own uC (STM32WLE5JC) comes by default with an AT command interface. This interface will be used by this library to connect with any Arduino board. It implements LoRa protocol.

## Library installation
### ArduinoIDE
You can download the library and copy it to the typical library path for Arduino IDE:
* **Windows**: 
    ```[USER ACCOUNT PATH]\Documents\Arduino\libraries\```
* **Linux**:
    ```/home/[USER NAME]/Arduino/libraries/```

### PlatformIO
As Arduino, download the library and copy it to the `lib` folder in your PlatformIO project


## Getting started
As any project, the best way to begin and understand the way this module and library works is trying out a simple example.

In [examples](examples/) folder you have a pair de examples.

The difference between both of them is the board where the library was tested, with their particular configurations

The highlights of the code are as follows:

### Linking and initialization
To play with the library funcionalities, just add the next `include`:
```C++
#include "Wio-E5.h"
```

To initialize the object, you must fill a configuration structure with mandatory parameters who define the LoRa connection, at physical and logical level. By the other hand, you must specify what Serial port will be used to connect with Wio-E5 module and -optionally- what Serial port will be used to show debug messages.
```C++
    BlackDevice::Wio_E5::Config_t lora_cfg;
    .
    .
    .

    lora_cfg = BlackDevice::Wio_E5::Default_Cfg;
    
    lora_cfg.reset_pin = WIO_E5_RESET_PIN;
    lora_cfg.band = default_lora_band;
    lora_cfg.appEui = appEui;
    lora_cfg.devEui = devEui;
    lora_cfg.appKey = appKey;

    lora = new BlackDevice::Wio_E5(&lora_cfg, serial_wio_e5, serial_monitor);
    lora->Begin();
```

#### Physical configuration
This library needs only 3 wires (plus ground) to connect Arduino host with Wio-E5 module:
* **Reset Pin**
* **UART Tx**
* **UART Rx**

Reset pin must be indicated in the configuration structure:
```C++
    lora_cfg.reset_pin = WIO_E5_RESET_PIN;
```

Serial Port is passed as a pointer in the constructor object. Optionally, you can indicate a second serial port to show debug messages generated from library.

Keep in mind that the library will initialize the serial port used to communicate with the module but debug serial port must be initialized externally due to use in the rest of the app code:
```C++
    serial_wio_e5 = &Serial; // port used for LoRa Module
    serial_monitor = &Serial1; // port used for debug
    serial_monitor->begin(115200, SERIAL_8N1);
    .
    .
    .
    lora = new BlackDevice::Wio_E5(&lora_cfg, serial_wio_e5, serial_monitor);
    lora->Begin();
```

#### Logical configuration
To connect to a LoraWan network, LoRa devices needs 3 different keys:
* **App EUI**
* **Device EUI**
* **App Key**

You must indicate all of them in the configuration structure:
```C++
    const char *appEui = "0123456789abcdef";
    const char *devEui = "22446688aabbccdd";
    const char *appKey = "9876543210abcdef9876543210abcde0";
    .
    .
    .    
    lora_cfg = BlackDevice::Wio_E5::Default_Cfg;
    lora_cfg.appEui = appEui;
    lora_cfg.devEui = devEui;
    lora_cfg.appKey = appKey;
```

Because LoRa network can work in different radio bands according to the country, you must indicate it in confguration structure too:
```C++
    const BlackDevice::Wio_E5::Band_t default_lora_band = BlackDevice::Wio_E5::Band_EU868;
    .
    .
    .
    lora_cfg = BlackDevice::Wio_E5::Default_Cfg;
    lora_cfg.band = default_lora_band;
```

## Compatible boards
This library was tested in the classical [Arduino Mega 2560](https://store.arduino.cc/products/arduino-mega-2560-rev3) and in a [WiLoBLino board](https://blackdevice.com/producto/wiloblino/).

In the first case, [WiLoBLino board](https://blackdevice.com/producto/wiloblino/) is used as a board hat using only the LoRa module

In the second case, [WiLoBLino board](https://blackdevice.com/producto/wiloblino/) is used as stand-alone board, executing the sketch in the embedded ESP32 module.

As you can define the Serial port and RESET pin the library will use, you can use it over any board you want. Of course, under your responsability.
