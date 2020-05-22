# Main script for recieving transmissions from Smartprobes.  Logs entries both locally and remotely.
# Be sure to run using python3!

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
#import adafruit_requests as requests
import requests
# Import python mysql connector
import mysql.connector

# Buttons could be used for say, a reset
# Button A
# btnA = DigitalInOut(board.D5)
# btnA.direction = Direction.INPUT
# btnA.pull = Pull.UP

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
    # draw a box to clear the LCD
    display.fill(0)
    display.text('RasPi LoRa', 35, 0, 1)

    # check for packet recpection
    packet = rfm9x.receive()
    if packet is None:
        display.show()
        display.text('- Waiting for PKT -', 15, 20, 1)

        # TESTING PARAGRAPH
	#scrubbed text is a test sequence used when you don't want to set up a Feather to send a message
        scrubbed_text = '{"weight":"yourmom", "temp":"9001", "time":"24:24", "id":"0", "flow":"112", "maxtemp":"42.0"}'
        #des_packet = json.loads(scrubbed_text)
        #print(des_packet)
        #r=requests.get("http://web.engr.oregonstate.edu/~veselyv/new.php", params=des_packet)
        #print(r.url)
        #command = "INSERT INTO sap (Timestamp, flow, weight, temperature, time, id, maxtemp) VALUES (now(),%s,%s,%s,%s,%s,%s)" 
        #values = ("0","0","0","24:24","0","0")
        #db_cursor.execute(command,values)
        #mysapflow.commit() 
        #time.sleep(10)

    else:
        # Display the packet text and rssi
        display.fill(0)
        prev_packet = packet
        print(prev_packet)
        # packet_text = str(prev_packet, "utf-8")
        # prev_packet = bytes('abcdeabcde\x00\x912\x00\r+\xd1\x91u\xbc\x00',encoding='utf-8')
        packet_text = str(prev_packet,encoding='utf-8',errors='replace')
        # display text
        display.text('RX: ', 0, 0, 1)
        # display.text(packet_text, 25, 0, 1)

        # Parse text correctly - remove extra chars at the end and format to JSON
        find_char = "}"
        remove_end = len(packet_text) - packet_text.find(find_char) - 1
        print(packet_text)
        scrubbed_text = packet_text[:-remove_end]
        print(scrubbed_text)
        # scrubbed_text = '{"weight":"yourmom", "temp":"9001", "time":"24:24", "id":"0", "flow":"112", "maxtemp":"42.0"}'
        des_packet = json.loads(scrubbed_text)
        print(des_packet)
        
        # Send packet to remote server
        #requests.get("http://web.engr.oregonstate.edu/~veselyv/new.php",weight=des_packet["weight"],temp=des_packet["temp"],time=des_packet["time"],id=des_packet["id"],flow=des_packet["flow"],maxtemp=des_packet["maxtemp"])
        r=requests.get("http://web.engr.oregonstate.edu/~veselyv/new.php", params=des_packet)
        print(r.url)


        # Log packet locally
        command = "INSERT INTO sap (Timestamp, flow, weight, temperature, time, id, maxtemp) VALUES (now(),%s,%s,%s,%s,%s,%s)" 
        values = (des_packet["flow"],des_packet["weight"],des_packet["temp"],des_packet["time"],des_packet["id"],des_packet["maxtemp"])
        db_cursor.execute(command,values)
        mysapflow.commit() 

        time.sleep(1)

    display.show()
    time.sleep(0.1)
