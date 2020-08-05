/*-
 * ========================START=================================
 * UCGDisplay :: Native :: Graphics
 * %%
 * Copyright (C) 2018 - 2020 Universal Character/Graphics display library
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU General Lesser Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/lgpl-3.0.html>.
 * =========================END==================================
 */
#ifndef UCGD_MOD_GRAPHICS_TYPES_H
#define UCGD_MOD_GRAPHICS_TYPES_H

#include <cstring>
#include <functional>
#include <map>
#include <iostream>
#include <Log.h>
#include <Global.h>

#if defined(__APPLE__) && !defined(__AVAILABILITY__)
#include <Availability.h>
#endif

//The <any> header for Mac OSX i386 is only available from the experimental namespace
#if defined(__APPLE__) && defined(__MAC_10_13) && !defined(__MAC_10_15)
#include <experimental/any>
typedef std::map<std::string, std::experimental::any> option_map_t;
#else

#include <any>

typedef std::map<std::string, std::any> option_map_t;
#endif

extern "C" {
#include <u8g2.h>
}

//Global macros
#define PROVIDER_LIBGPIOD "libgpiod"
#define PROVIDER_CPERIPHERY "cperiphery"
#define PROVIDER_PIGPIO "pigpio" //pigpio - standalone
#define PROVIDER_PIGPIOD "pigpiod" //pigpio - daemon
#define PROVIDER_DEFAULT PROVIDER_CPERIPHERY
/**
 * C linked against pigpio. Fastest code, slowest development.
 * Only one program linked against the pigpio library can be running at a time (the program in effect becomes the pigpio daemon).
 * Program can only run on the Raspberry Pi.
 *
 * @see https://github.com/joan2937/pigpio/issues/129#issuecomment-300442282
 */
#define PIGPIO_TYPE_STANDALONE 0

/**
 * C linked against pigpiod_if2 (via pigpio daemon): Fast code, fast development. Requires the pigpio daemon to be running.
 * Many programs can be running at any one time talking to the pigpio daemon.
 * Programs can be compiled and run on any machine with a suitable compiler, e.g. Linux, Windows, Mac.
 *
 * @see https://github.com/joan2937/pigpio/issues/129#issuecomment-300442282
 */
#define PIGPIO_TYPE_DAEMON 1

//Options
#define OPT_ROTATION "rotation"
#define OPT_BUS_SPEED "bus_speed"
#define OPT_PROVIDER "default_provider"
#define OPT_GPIO_CHIP "gpio_chip"

#define OPT_PROVIDER_GPIO "provider_gpio"
#define OPT_PROVIDER_SPI "provider_spi"
#define OPT_PROVIDER_I2C "provider_i2c"

#define OPT_SPI_CHANNEL "spi_channel"
#define OPT_SPI_BUS "spi_bus_number"
#define OPT_SPI_FLAGS "spi_flags"
#define OPT_SPI_MODE "spi_mode"
#define OPT_SPI_BIT_ORDER "spi_bit_order"
#define OPT_SPI_BITS_PER_WORD "spi_bits_per_word"

#define OPT_I2C_BUS "i2c_bus_number"
#define OPT_I2C_FLAGS "i2c_flags"
#define OPT_I2C_ADDRESS "i2c_address"

//Pigpio specific options
#define OPT_PIGPIO_TYPE "pigpio_mode"
#define OPT_PIGPIO_ADDR "pigpio_addr"
#define OPT_PIGPIO_PORT "pigpio_port"

//Misc options
#define OPT_EXTRA_DEBUG_INFO "extra_debug_info"

/*
 * -------------------------------------------------------------------------------------------------------------
 * Note: To enable SPI Auxillary channel on the Raspberry Pi, you need to enable it on the device tree overlay
 * -------------------------------------------------------------------------------------------------------------
 * 1. Add the following to /boot/config.txt
 *
 * # enable spi1 with a single CS line
 *    dtoverlay=spi1-1cs*
 *
 * # enable spi1 with two CS lines
 *    dtoverlay=spi1-2cs*
 *
 * # enable spi1 with three CS lines
 *    dtoverlay=spi1-3cs
 *
 * 2. Verify
 *
 *   lsmod | grep spi
 * -------------------------------------------------------------------------------------------------------------
*/

//Available SPI Peripherals (Main & Auxillary) on the Raspberry Pi
#define SPI_PERIPHERAL_MAIN 0
#define SPI_PERIPHERAL_AUX  1

//Fixed Raspberry Pi Pins for Main SPI Peripheral
#define SPI_RPI_PIN_MAIN_MOSI 10
#define SPI_RPI_PIN_MAIN_MISO 9
#define SPI_RPI_PIN_MAIN_SCLK 11
#define SPI_RPI_PIN_MAIN_CE0 8
#define SPI_RPI_PIN_MAIN_CE1 7

//Fixed Raspberry Pi Pins for Auxillary SPI Peripheral
#define SPI_RPI_PIN_AUX_MOSI 20
#define SPI_RPI_PIN_AUX_MISO 19
#define SPI_RPI_PIN_AUX_SCLK 21
#define SPI_RPI_PIN_AUX_CE0 18
#define SPI_RPI_PIN_AUX_CE1 17
#define SPI_RPI_PIN_AUX_CE2 16

//SPI Channels for the Raspberry Pi (Chip-Select)
#define SPI_RPI_CHANNEL_CE0 0
#define SPI_RPI_CHANNEL_CE1 1
#define SPI_RPI_CHANNEL_CE2 2

//Fixed Raspberry Pi Pins for I2C Peripheral
#define I2C_RPI_PIN_SDA 2
#define I2C_RPI_PIN_SCL 3

//Fixed Raspberry Pi Pins for UART Peripheral
#define UART_RPI_PIN_TXD 14
#define UART_RPI_PIN_RXD 15

//The communication method (Hardware or Software)
#define COMTYPE_HW 0
#define COMTYPE_SW 1

//The communication interfaces (name/value pairs are idential to the u8g2 macros)
#define COMINT_4WSPI 0x0001
#define COMINT_3WSPI 0x0002
#define COMINT_6800  0x0004
#define COMINT_8080 0x0008
#define COMINT_I2C  0x0010
#define COMINT_ST7920SPI 0x0020     /* mostly identical to COM_4WSPI, but does not use DC */
#define COMINT_UART 0x0040
#define COMINT_KS0108  0x0080        /* mostly identical to 6800 mode, but has more chip select lines */
#define COMINT_SED1520  0x0100

//Uncomment to enable debbugging
#define DEBUG_UCGD

class OptionNotFoundException : public std::runtime_error {
public:
    explicit OptionNotFoundException(const std::string &arg) : runtime_error(arg) {}

    explicit OptionNotFoundException(const char *string) : runtime_error(string) {}

    explicit OptionNotFoundException(const runtime_error &error) : runtime_error(error) {}
};

//Forward declarations
class UcgdProvider;

class UcgdSpiPeripheral;

class UcgdI2CPeripheral;

class UcgdGpioPeripheral;

struct ucgd_t;

#if (defined(__arm__) || defined(__aarch64__)) && defined(__linux__)

struct cp_spi_handle {
    int fd;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

struct cp_i2c_handle {
    int fd;

    struct {
        int c_errno;
        char errmsg[96];
    } error;
};

#include <spi.h>
#include <i2c.h>

#endif

typedef std::function<uint8_t(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)> u8g2_msg_func_t;

typedef std::function<void(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb,
                           u8x8_msg_cb gpio_and_delay_cb)> u8g2_setup_func_t;

typedef std::map<std::string, u8g2_setup_func_t> u8g2_setup_func_map_t;

typedef std::map<std::string, const uint8_t *> u8g2_lookup_font_map_t;

typedef std::function<uint8_t(const std::shared_ptr<ucgd_t> &info, u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                              void *arg_ptr)> u8g2_msg_func_info_t;

typedef struct {
    //pin configuration
    int d0 = -1; //spi-clock
    int d1 = -1; //spi-data
    int d2 = -1;
    int d3 = -1;
    int d4 = -1;
    int d5 = -1;
    int d6 = -1;
    int d7 = -1;
    int en = -1;
    int cs = -1;
    int dc = -1;
    int reset = -1;
    int scl = -1;
    int sda = -1;
    int cs1 = -1;
    int cs2 = -1;
} u8g2_pin_map_t;

/**
 * The context
 */
struct ucgd_t {
    //U8g2 pin mapping definition
    u8g2_pin_map_t pin_map;
    //U8g2 descriptor
    std::unique_ptr<u8g2_t> u8g2;
    //U8g2 setup procedure name
    std::string setup_proc_name;
    //U8g2 Setup Callback
    u8g2_setup_func_t setup_cb;
    //BYTE callback
    u8g2_msg_func_t byte_cb;
    //GPIO callback
    u8g2_msg_func_t gpio_cb;
    //Dislpay rotation mode
    u8g2_cb_t *rotation;
    //font flag
    bool flag_font;
    //virtual flag
    bool flag_virtual;
    //communications interface
    int comm_int;
    //communications type
    int comm_type;
    //debug flag
    bool debug;
    //pixel buffer
    uint8_t* buffer;
    long bufferSize;
    //pixel buffer (bgra)
    uint8_t* bufferBgra;
    long bufferBgraSize;

    ~ucgd_t() {
        ::debug(std::string("ucgd_t : Device closed: ") + std::to_string(address()));
    }

    uintptr_t address() {
        return (uintptr_t) u8g2.get();
    }

    //Only available on ARM 32/64 bit platforms
#if (defined(__arm__) || defined(__aarch64__)) && defined(__linux__)
    //spi handle for third-party providers
    int tp_spi_handle = -1;
    //i2c handle for third party providers
    int tp_i2c_handle = -1;
    //the system spi handle (c-periphery)
    std::unique_ptr<cp_spi_t> sys_spi_handle;
    //the system i2c handle (c-periphery)
    std::unique_ptr<cp_i2c_t> sys_i2c_handle;
    //options associated with this context
    std::map<std::string, std::any> options;

    auto setDefaultProvider(std::shared_ptr<UcgdProvider> &provider) -> void {
        this->provider = provider;
    }

    auto getDefaultProvider() -> std::shared_ptr<UcgdProvider> {
        auto prov = provider.lock();
        if (prov)
            return prov;
        throw std::runtime_error("Provider no longer available");
    }

    std::any &getOption(const std::string &key) {
        auto it = this->options.find(key);
        if (it != this->options.end()) {
            return it->second;
        }
        throw OptionNotFoundException(std::string("Key '") + key + std::string("' not found in options"));
    }

    std::string getOptionString(const std::string &key, std::string defaultVal = "") {
        try {
            std::any value = getOption(key);
            return std::any_cast<std::string>(value);
        } catch (OptionNotFoundException &e) {
            return defaultVal;
        }
    }

    int getOptionInt(const std::string &key) {
        std::any value = getOption(key);
        return std::any_cast<int>(value);
    }

    int getOptionInt(const std::string &key, int defaultVal) {
        try {
            std::any value = getOption(key);
            return std::any_cast<int>(value);
        } catch (OptionNotFoundException &e) {
            return defaultVal;
        }
    }

    [[nodiscard]] const std::weak_ptr<UcgdProvider> &getProvider() const {
        return provider;
    }

private:
    std::weak_ptr<UcgdProvider> provider;
#endif
};

#endif //UCGD_MOD_GRAPHICS_TYPES_H
