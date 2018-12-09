### Universal Character/Graphics Display Library for Java

[![Build Status](https://travis-ci.org/ribasco/ucgdisplay.svg?branch=master)](https://travis-ci.org/ribasco/ucgdisplay) [![Maven Central](https://img.shields.io/maven-central/v/com.ibasco.ucgdisplay/ucg-display.svg?label=Maven%20Central)](https://search.maven.org/search?q=com.ibasco.ucgdisplay) [![Join the chat at https://gitter.im/ucgdisplay/Lobby](https://badges.gitter.im/ucgdisplay/Lobby.svg)](https://gitter.im/ucgdisplay/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/4209cfcd33eb4f98a6e1d16414804d45)](https://app.codacy.com/app/ribasco/ucgdisplay?utm_source=github.com&utm_medium=referral&utm_content=ribasco/ucgdisplay&utm_campaign=Badge_Grade_Dashboard) [![Javadocs](https://www.javadoc.io/badge/com.ibasco.ucgdisplay/ucg-display.svg)](https://www.javadoc.io/doc/com.ibasco.ucgdisplay/ucg-display)

### Features

---

##### General features

* Supports both Character and Graphics/Dot-Matrix display devices
* Should work for most linux based SBCs (kernel 4.8x or higher)
 
##### Display drivers

######  Character LCD driver features
* Pure java implementation for Hitachi HD44780 driver powered by Pi4j
* Flexible configuration options for interfacing with your SBC device (e.g. GPIO expanders/I2C/SPI)
* No fixed/mandatory pin mapping. You have the freedom to choose whatever device pins you want to use for your LCD device.
* Available LCD adapters
    * MCP23017
    * Shift Register (e.g. 74HC595)
    * GPIO
    * MCP23008 (Coming soon)
    * PCF8574 (Coming soon)
###### Graphic LCD driver features
* SPI and I2C hardware capability and other software bitbanging implementations
* Over 46+ controllers are supported. Refer to the table below for the list of supported display controllers.
* The graphics display module wraps around the popular c/c++ [U8g2](https://github.com/olikraus/u8g2) library by Oliver. All drawing operations present in the library should be similar to the ones found in U8g2 (Refer to the official [U8g2 reference manual](https://github.com/olikraus/u8g2/wiki/u8g2reference) for more information). 

#####  Event-driven UI framework (Coming soon) 

### Supported display controllers

---
###### Character LCD

* Hitachi HD44780
    
###### Graphic/Dot-Matrix LCD

* A2PRINTER, HX1230, IL3820, IST3020, KS0108, LC7981, LD7032, LS013B7DH03, LS027B7DH01, MAX7219, NT7534, PCD8544, PCF8812, RA8835, SBN1661, SED1330, SED1520, SH1106, SH1107, SH1108, SH1122, SSD1305, SSD1306, SSD1309, SSD1317, SSD1322, SSD1325, SSD1326, SSD1327, SSD1329, SSD1606, SSD1607, ST75256, ST7565, ST7567, ST7586S, ST7588, ST7920, T6963, UC1601, UC1604, UC1608, UC1610, UC1611, UC1638, UC1701

### Supported bus interfaces (for glcd)

| Interface     | Type              | Description                                                                       | Hardware | Software |
|---------------|-------------------|-----------------------------------------------------------------------------------|----------|----------|
| SPI           | 4-Wire            | 4-Wire interface                                                                  | YES      | YES      |
| SPI           | 4-Wire (ST7920)   | 4-Wire interface with no DC                                                       | YES      | YES      |
| SPI           | 3-Wire            | 3-Wire interface                                                                  | NO       | YES      |
| I2C           | Two-Wire          | I2C Two-Wire interface                                                            | YES      | YES      |
| Parallel 8080 | Parallel          | Parallel 8080 (Intel)                                                             | NO       | YES      |
| Parallel 6800 | Parallel          | Parallel 6800 (Motorola)                                                          | NO       | YES      |
| Parallel 6800 | Parallel (KS0108) | Special KS0108 protocol. Mostly identical to 6800, but has more chip select lines | NO       | YES      |
| SED1520       | SED1520           | Special SED1520 protocol                                                          | NO       | YES      |


### Project Resources

---
* [Snapshot Builds](https://oss.sonatype.org/content/repositories/snapshots/com/ibasco/ucgdisplay/)
* [Release Builds](https://oss.sonatype.org/content/repositories/releases/com/ibasco/ucgdisplay)
* [Display Controller Status](https://goo.gl/5GNQmy) - Please feel free to edit this page if you were able to successfully test your device using this library

### Pre-requisites

---
* Java JDK 1.8 or higher
* [Libgpiod](https://github.com/brgl/libgpiod) - C library and tools for interacting with the linux GPIO character device (gpiod stands for GPIO device)
 
### Installation

---


1. From maven repository

    ```xml
    <dependencies>
        <!-- Character display driver (HD44780) -->
         <dependency>
             <groupId>com.ibasco.ucgdisplay</groupId>
             <artifactId>ucgd-drivers-clcd</artifactId>
             <version>1.3.0-alpha</version>
         </dependency>
      
         <!-- Graphics display driver -->
         <dependency>
             <groupId>com.ibasco.ucgdisplay</groupId>
             <artifactId>ucgd-drivers-glcd</artifactId>
             <version>1.3.0-alpha</version>
         </dependency>
    </dependencies>
    ```
    
2. From source

    Clone from github

    ```bash
    git clone https://github.com/ribasco/ucgdisplay.git
    ```        

    Switch to the project directory

    ```bash
    cd ucgdisplay
    ```

    Install to your local maven repository

    ```bash
    mvn install
    ```

### Usage examples

---
######  Character LCD Example (HD44780)

Simple hello world example demonstrating the use of different LCD adapters.

```java
package com.ibasco.ucgdisplay.examples;

import com.ibasco.ucgdisplay.drivers.clcd.HD44780DisplayDriver;
import com.ibasco.ucgdisplay.drivers.clcd.LcdGpioAdapter;
import com.ibasco.ucgdisplay.drivers.clcd.LcdPinMapConfig;
import com.ibasco.ucgdisplay.drivers.clcd.LcdTemplates;
import com.ibasco.ucgdisplay.drivers.clcd.adapters.GpioLcdAdapter;
import com.ibasco.ucgdisplay.drivers.clcd.adapters.Mcp23017LcdAdapter;
import com.ibasco.ucgdisplay.drivers.clcd.adapters.ShiftRegisterLcdAdapter;
import com.ibasco.ucgdisplay.drivers.clcd.enums.LcdPin;
import com.ibasco.ucgdisplay.drivers.clcd.providers.MCP23017GpioProviderExt;
import com.pi4j.io.gpio.GpioFactory;
import com.pi4j.io.gpio.RaspiPin;
import com.pi4j.io.i2c.I2CBus;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HD44780Example {

    private static final Logger log = LoggerFactory.getLogger(HD44780Example.class);

    public static void main(String[] args) {
        try {
            LcdPinMapConfig config = new LcdPinMapConfig()
                    .map(LcdPin.RS, RaspiPin.GPIO_02)
                    .map(LcdPin.EN, RaspiPin.GPIO_03)
                    .map(LcdPin.DATA_4, RaspiPin.GPIO_04)
                    .map(LcdPin.DATA_5, RaspiPin.GPIO_05)
                    .map(LcdPin.DATA_6, RaspiPin.GPIO_06)
                    .map(LcdPin.DATA_7, RaspiPin.GPIO_07);
            
            //GPIO adapter
            LcdGpioAdapter adapter = new GpioLcdAdapter(config);

            //Shift register adapter
            //LcdGpioAdapter adapter = new ShiftRegisterLcdAdapter(GpioFactory.getDefaultProvider(), RaspiPin.GPIO_02, RaspiPin.GPIO_03, RaspiPin.GPIO_04, config);

            //MCP23017 I2C adapter (Using built-in templates)
            //MCP23017GpioProviderExt mcp23017GpioProvider = new MCP23017GpioProviderExt(I2CBus.BUS_1, 0x15);
            //LcdGpioAdapter adapter = new Mcp23017LcdAdapter(mcp23017GpioProvider, LcdTemplates.ADAFRUIT_I2C_RGBLCD_MCP23017);
            
            HD44780DisplayDriver charDriver = new HD44780DisplayDriver(adapter, 20, 4);
            charDriver.home();
            charDriver.write("Hello World");
        } catch (Exception e) {
            log.error("Error occured", e);
        }
    }
}

```

######  Graphic LCD Example (ST7920)

Simple hello world example for ST7920 controller. In this example, we use the Raspberry Pi SPI hardware features. Please note that in Raspberry Pi, we need to set the pin modes to ALT0 to activate the hardware features (also applicable for the I2C pins). This library does not automatically set the correct pin modes for you, so you have to explicitly set them prior to running the application (example below).  

```java
package com.ibasco.ucgdisplay.examples;

import com.ibasco.ucgdisplay.drivers.glcd.*;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdBusInterface;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdFont;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdPin;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdRotation;

public class GlcdST7920Example {
    public static void main(String[] args) {
        //On raspberry pi, we use the bcm pin numbering scheme (See: https://elinux.org/RPi_BCM2835_GPIOs)
        GlcdConfig config = GlcdConfigBuilder.create()
                .rotation(GlcdRotation.ROTATION_NONE)
                .busInterface(GlcdBusInterface.SPI_HW_4WIRE_ST7920)
                 //SPI = /dev/spidev0.0 or /dev/spidev0.1
                 //I2C = /dev/i2c-0 or /dev/i2c-1
                .transportDevice("/dev/spidev0.0")
                //The GPIO character device chip (Execute 'ls -l /dev/gpiochip*' to list all available chips)  
                .gpioDevice("/dev/gpiochip0")                
                .display(Glcd.ST7920.D_128x64)
                //Pin mapping (alternatively, you could use #mapPin(GlcdPin, int) function) 
                .pinMap(new GlcdPinMapConfig()
                        .map(GlcdPin.SPI_CLOCK, 14)
                        .map(GlcdPin.SPI_MOSI, 12)
                        .map(GlcdPin.CS, 10)
                )
                .build();

        GlcdDriver driver = new GlcdDriver(config);


        driver.setCursor(0, 10);
        driver.setFont(GlcdFont.FONT_6X12_MR);
        driver.clearBuffer();
        driver.drawString("Hello World");
        driver.sendBuffer();
    }
}

```

**Setting pin mode (using wiring pi)**

```
gpio mode 12 alt0 && gpio mode 13 alt0 && gpio mode 14 alt0 && gpio mode 10 alt0 && gpio readall
```

**Verify**

use `gpio readall` to verify that we have set the correct mode (ALT0) for MOSI, SCLK and CE0 (Chip-Select)

```shell
 +-----+-----+---------+------+---+---Pi 2---+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 | ALT0 | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 | ALT0 | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | ALT0 | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | ALT0 | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 1 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 1 | OUT  | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | OUT  | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 2---+---+------+---------+-----+-----+

```

### Limitations

---
* Works only on linux kernel v4.8 or higher
* The adapters for character lcd driver Mcp23008LcdAdapter and Pcf8574TLcdAdapter are not yet implemented. 

### Contribution guidelines

---
* Build instructions (Coming soon)

### Related projects

---
* [GLCD Simulator Project](https://github.com/ribasco/glcd-emulator)
