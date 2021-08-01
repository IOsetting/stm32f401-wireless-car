
# Hardware

## Controller

![](https://img2020.cnblogs.com/blog/650273/202108/650273-20210801185045242-1887598038.jpg)

* Power supply: 3.7V 18650 battery
* 3.3V: 1N4148 x 1
* MCU: STM32F401CCU6 Development board
* 2-axis joystick potentiometer
* Wireless: nRF24L01


## Car

![](https://img2020.cnblogs.com/blog/650273/202108/650273-20210801185022034-445886525.jpg)

* Power supply: 7.4V (18650 battery x 2)
* 6V: IN4007 x 2
* MCU: STM32F401CCU6
* Motor driver: L9110 dual-channel x2
* 48:1 motor x 4


# Connections

## Controller


* UART: 
   * PA9 => USB2TTL的RX
   * PA10 => USB2TTL的TX
* SPI: nRF24L01
   * PA5,PA6,PA7, PB13,PB14,PB15
* ADC: joystick potentiometer
   * PA0 => AXIS X
   * PA1 => AXIS Y
* VCC
* GND

nRF24L01 connections

| STM32          |  nRF24L01 |
| -------------- | --------- |
| PA4 SPI1_NSS   |  N/A      |
| PA5 SPI1_SCK   |  SCK      |
| PA6 SPI1_MISO  |  MISO     |
| PA7 SPI1_MOSI  |  MOSI     |
| PB13           |  IRQ      |
| PB14           |  CE       |
| PB15           |  CSN      |

## Car


* UART: 
   * PA9,PA10
* SPI: nRF24L01
   * PA5,PA6,PA7, PB13,PB14,PB15
* PWM: 4 pin for 4 PWM outpus
   * PA0,PA2: left motors
   * PA1,PA3: right motors

nRF24L01 connections are the same as controller.


# Reference

* https://learn.adafruit.com/improve-brushed-dc-motor-performance?view=all
