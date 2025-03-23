#!/usr/bin/python
import RPi.GPIO as GPIO
import pigpio
import time
import threading
import random
import optparse
from enum import Enum

parser = optparse.OptionParser()

parser.add_option('-f', '--frequency', dest="frequency", default=1000000,
                  help="set target frequency in hz (default: 1Mhz)", 
                  type=float)
parser.add_option('-d', '--display-ratio', dest="display_ratio", default=50000,
                  help="How long to hold before shifting in new input (default: 50000)", 
                  type=float)

options, args = parser.parse_args()
period = 1 / options.frequency

SRCLK_GPIO=17
SER_GPIO=27
OE_GPIO=22
RCLK_GPIO=23
SRCLR_GPIO=24

pi = pigpio.pi()

def gpio_init(gpio_pin):
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(gpio_pin,GPIO.OUT)

def gpio_setup():
    gpio_init(SRCLK_GPIO)
    gpio_init(SER_GPIO)
    gpio_init(OE_GPIO)
    gpio_init(RCLK_GPIO)
    gpio_init(SRCLR_GPIO)

def led_on(gpio_pin):
    GPIO.output(gpio_pin,GPIO.HIGH)

def led_off(gpio_pin):
    GPIO.output(gpio_pin,GPIO.LOW)

def blink(delay, gpio_pin):
    while 1:
        led_on(gpio_pin)
        time.sleep(delay)
        led_off(gpio_pin)
        time.sleep(delay)

def random_colors():
    last_color = 1
    color = 2
    while 1:
        led_on(OE_GPIO) #active low
        led_off(RCLK_GPIO)
        while color == last_color or color == 0:
            color = random.randint(0,7)
        output_bitmap = 0
        if (color & 0x1):
            output_bitmap |= 0xff0000
        if (color & 0x2):
            output_bitmap |= 0xff00
        if (color & 0x4):
            output_bitmap |= 0xff
        for i in range(24):
            if (output_bitmap & (1 << i)):
                led_on(SER_GPIO)
            else:
                led_off(SER_GPIO)
            led_on(RCLK_GPIO)
            time.sleep(period)
            led_off(RCLK_GPIO)
            time.sleep(period)
        led_on(RCLK_GPIO)
        led_off(OE_GPIO) #active low
        time.sleep(period * options.display_ratio)
        last_color = color

def ser_input(bitmap_list):
    while 1:
        led_on(OE_GPIO) #active low
        led_off(RCLK_GPIO)
        for bitmap in bitmap_list:
            for i in range(24):
                if (bitmap & (1 << i)):
                    led_on(SER_GPIO)
                else:
                    led_off(SER_GPIO)
                led_on(RCLK_GPIO)
                time.sleep(period)
                led_off(RCLK_GPIO)
                time.sleep(period)
            led_on(RCLK_GPIO)
            led_off(OE_GPIO) #active low
            time.sleep(period * options.display_ratio)

def chaser():
    last_color = 1
    color = 2
    while 1:
        led_on(OE_GPIO) #output disabled
        led_off(RCLK_GPIO)
        bitmap_list = []
        bitmap = 0
        while color == last_color or color == 0:
            color = random.randint(0,7)
        if (color & 0x1):
            bitmap |= 0x10000
        if (color & 0x2):
            bitmap |= 0x100
        if (color & 0x4):
            bitmap |= 0x1
        for i in range(8):
            bitmap_list += [bitmap<<i]       
        for bitmap in bitmap_list:
            for i in range(24):
                if (bitmap & (1 << i)):
                    led_on(SER_GPIO)
                else:
                    led_off(SER_GPIO)
                led_on(RCLK_GPIO)
                led_on(SRCLK_GPIO)
                time.sleep(period)
                led_off(RCLK_GPIO)
                led_off(SRCLK_GPIO)
                time.sleep(period)
            #led_on(RCLK_GPIO)
            led_off(OE_GPIO) #output enabled
            for i in range(int(options.display_ratio)):
                led_on(SRCLK_GPIO)
                time.sleep(period)
                led_off(SRCLK_GPIO)
                time.sleep(period)
        last_color = color

def simple_chaser():
    bitmap_list = []
    for i in range(8):
        bitmap_list += [1<<i]       
    while 1:
        led_on(OE_GPIO) #output disabled
        led_off(RCLK_GPIO)
        for bitmap in bitmap_list:
            for i in range(24):
                if (bitmap & (1 << i)):
                    led_on(SER_GPIO)
                else:
                    led_off(SER_GPIO)
                led_on(RCLK_GPIO)
                led_on(SRCLK_GPIO)
                time.sleep(period)
                led_off(RCLK_GPIO)
                led_off(SRCLK_GPIO)
                time.sleep(period)
            #led_on(RCLK_GPIO)
            led_off(OE_GPIO) #output enabled
            for i in range(int(options.display_ratio)):
                led_on(SRCLK_GPIO)
                time.sleep(period)
                led_off(SRCLK_GPIO)
                time.sleep(period)
 
            
def run():
    
    # start the clock
    #t = threading.Thread(target=blink, args=(period, SRCLK_GPIO))
    #t.start()

    # Deassert and assert clear (active low)
    led_off(SRCLR_GPIO)
    time.sleep(0.1)
    led_on(SRCLR_GPIO)

    #random_colors()

    # red, blue
    #bitmap_list = [0xff0000, 0x00ff00]
    #ser_input(bitmap_list)

    #chaser()

    simple_chaser()

    
#main
gpio_setup()
run()
pi.stop()
