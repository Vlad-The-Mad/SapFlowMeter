"""
Example for using the RFM9x Radio with Raspberry Pi.

Learn Guide: https://learn.adafruit.com/lora-and-lorawan-for-raspberry-pi
Author: Brent Rubell for Adafruit Industries
"""
# Import Python System Libraries
import time
# Import Blinka Libraries
import busio
from digitalio import DigitalInOut, Direction, Pull
import board
# Import the SSD1306 module.
import adafruit_ssd1306
# Import RFM9x
import adafruit_rfm9x
# Import JSON module
import json
# Import Circuit Python Requests modules
import adafruit_requests as requests
# Import python mysql connector
import mysql.connector

# Button A
btnA = DigitalInOut(board.D5)
btnA.direction = Direction.INPUT
btnA.pull = Pull.UP

# Create the I2C interface.
i2c = busio.I2C(board.SCL, board.SDA)

# 128x32 OLED Display
reset_pin = DigitalInOut(board.D4)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, reset=reset_pin)
# Clear the display.
display.fill(0)
display.show()
width = display.width
height = display.height

# Configure LoRa Radio
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0)
rfm9x.tx_power = 23
prev_packet = None

# Connect to local database
mysapflow = mysql.connector.connect(
host="localhost",
user="veselyv",
passwd="Amigos",
database="sapflow"
)
db_cursor = mysapflow.cursor()

while True:
    packet = None
    # draw a box to clear the image
    display.fill(0)
    display.text('RasPi LoRa', 35, 0, 1)

    # check for packet rx
    packet = rfm9x.receive()
    if packet is None:
        display.show()
        display.text('- Waiting for PKT -', 15, 20, 1)
    else:
        # Display the packet text and rssi
        display.fill(0)
        prev_packet = packet
        packet_text = str(prev_packet, "utf-8")
        find_char = "}"
        
        remove_end = 89 - packet_text.find(find_char)
        scrubbed_text = packet_text[:-remove_end]
        display.text('RX: ', 0, 0, 1)
        display.text(packet_text, 25, 0, 1)
        des_packet = json.loads(scrubbed_text)
        print(des_packet)

        # Send packet to remote server
        #requests.get("http://web.engr.oregonstate.edu/~veselyv/new.php",weight=des_packet["weight"],temp=des_packet["temp"],time=des_packet["time"],id=des_packet["id"],flow=des_packet["flow"],maxtemp=des_packet["maxtemp"])
        # requests.get("http://web.engr.oregonstate.edu/~veselyv/new.php")

        # Log packet locally
        command = "INSERT INTO sap (Timestamp, flow, weight, temperature, time, id, maxtemp) VALUES (now(),%s,%s,%s,%s,%s,%s)" 
        values = (des_packet["flow"],des_packet["weight"],des_packet["temp"],des_packet["time"],des_packet["id"],des_packet["maxtemp"])
        db_cursor.execute(command,values)
        mysapflow.commit() 

        time.sleep(1)

#    if not btnA.value:
       # Send Button A
#        display.fill(0)
#        button_a_data = bytes("Button A!\r\n","utf-8")
#        rfm9x.send(button_a_data)
#        display.text('Sent Button A!', 25, 15, 1)
#    elif not btnB.value:


    display.show()
    time.sleep(0.1)
