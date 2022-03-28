#******************************************************************************
# * @file           : ST_configFunction.py
# * @brief          : Contains support functions for quickConnect.py related to local files.
# ******************************************************************************
# * @attention
# *
# * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
# * All rights reserved.</center></h2>
# *
# * This software component is licensed by ST under Ultimate Liberty license
# * SLA0044, the "License"; You may not use this file except in compliance with
# * the License. You may obtain a copy of the License at:
# *                             www.st.com/SLA0044
# *
# ******************************************************************************
import sys
import serial
import serial.tools.list_ports
import platform
import time
import atexit
import os
import string
from BOTO3_configFunctions import *

# List of possible board labels
boards = ["DIS_L4IOT", "DIS_L4S5VI"]

# Determines USB path for the board provisioning process
def find_path(op_sys):
    USBPATH = ''
    if op_sys == "Windows":
        # Find drive letter
        for l in string.ascii_uppercase:
            if os.path.exists('%s:\\MBED.HTM' % l):
                USBPATH = '%s:\\' % l
                break
        
    elif op_sys == "Linux":
        user = os.getlogin()
        for board in boards:
            temp_path = '/media/%s/%s' % (user, board)
            if os.path.exists(temp_path):
                USBPATH = temp_path
                break
    elif op_sys == "Darwin": # Mac
        for board in boards:
                temp_path = '/Volumes/%s/' % board
                if os.path.exists(temp_path):
                    USBPATH = temp_path
                    break
    else:
        print("Operating System error")
        sys.exit()
    
    return USBPATH

# Flashes the provided file to the board connected at COM
# Parameters:
#   flashing_file: Path to .bin file.
#   USBPATH: Path to USB of connected board.


 
def flash_board(flashing_file, USBPATH, COM):

    port = serial.Serial(COM,115200)
    time.sleep(1)

    session_os = platform.system()

    # In Windows
    if session_os == "Windows":
        cmd = 'copy "'+flashing_file+'" "'+USBPATH+'File.bin"'
    else:
        cmd = 'cp "'+flashing_file+'" "'+USBPATH+'File.bin"'

    print(cmd)

    os.system(cmd)

    # Wait until data comes through is complete
    bytesToRead = port.in_waiting
    while (port.in_waiting <= bytesToRead):
        time.sleep(0.01)
    

# Updates the EEPROM with the supplied credentials in Config.txt
# Parameters:
#   COM: Serial port for the desired board.
def update_eeprom(COM):

    ser = serial.Serial(COM, 115200)

    ssid = ""
    pswd = ""
    endpt = ""

    with open("Config.txt", 'r') as input:
        for line in input:
            if 'SSID' in line:
                ssid = line.split()[-1] + '\r\n'
            if 'Password' in line: 
                pswd = line.split()[-1] + '\r\n'
            if 'Endpoint' in line:
                endpt = line.split()[-1] + '\r\n'

    print("Updating Endpoint")
    ser.write(b'1')
    wait(ser)
    ser.write(bytes(endpt, 'utf-8'))
    wait(ser)

    print("Updating SSID")
    ser.write(b'2')
    wait(ser)
    ser.write(bytes(ssid, 'utf-8'))
    wait(ser)

    print("Updating Password")
    ser.write(b'3')
    wait(ser)
    ser.write(bytes(pswd, 'utf-8'))
    wait(ser)

    print("Updating Wi-Fi Security to EEPROM")
    ser.write(b'4')
    wait(ser)
    ser.write(b'3')
    wait(ser)

    print("Writing Credentials to EEPROM")
    ser.write(b'5')
    wait(ser)

    ser.close()
    # Do not remove time.sleep(1). It is for Mac compatibility. 
    time.sleep(1)

# Waits until serial port has full message in_waiting.
def wait(ser):
    start = ser.in_waiting
    time.sleep(0.1)
    while ser.in_waiting > start:
        time.sleep(0.1)
        start = ser.in_waiting

# Finds and returns the port for the connected board.
def get_com():
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if "VID:PID=0483:374" in p.hwid:
            return p.device
    
    return " PORT ERR "
            

# Removes old certificate file and renames temp cert.pem file to thing_name_cert.pem
#
#Parameters: 
#   thing_name. String. Name of the thing associated with the temp cert
def prep_cert(thing_name):
    thing_cert = "./Certs/" + thing_name + "_cert.pem"

    if os.path.exists(thing_cert):
        os.remove(thing_cert)
    
    os.rename("./Certs/cert.pem", thing_cert)

# Saves the certificate of the board connected to COM to ./Certs/cert.pem
#
# Parameters: COM. Serial port the desired board is connected.
def get_cert_from_stm32(COM):
    ser = serial.Serial(COM, 115200)
    
    if not os.path.exists("./Certs/"):
        os.mkdir("./Certs/")

    cert = open("Certs/cert.pem",'w')

    PEM_BEGIN_CRT = "-----BEGIN CERTIFICATE-----\n"
    PEM_END_CRT   = "-----END CERTIFICATE-----\n"
    THING_NAME_IS_STRING = "Shadow Thing Name is"

    cert_start_received = False
    cert_end_received = False
    thin_name_found = False

    #subs = ""

    # Find the Thing certificate and save it to cert.pem
    while cert_end_received == False:
        line = ser.readline().decode("utf-8")
        print(line, end = '')

        if line == PEM_BEGIN_CRT:
            cert_start_received = True

        if line == PEM_END_CRT:
            cert_end_received = True  

        if cert_start_received == True:
            cert.write(line)

    cert.close() 

    # Find the Thing name and save it to thing_name.txt
    while thin_name_found == False:
        line = ser.readline().decode("utf-8")
        print(line, end = '')

        if THING_NAME_IS_STRING in line:
            thin_name_found = True

            try:
                index = line.index(THING_NAME_IS_STRING)
                idx1 = line.find('is')
                idx2 = line.find('(')
                subs = line[idx1+3:idx2-1]
            except ValueError:
                print("Not found!")
                
            save_config('ThingName', subs)

# Indefinite reads serial port for COM
#
# Parameters: COM. Serial port the desired board is connected.
def serial_reader(COM):
    ser = serial.Serial(COM, 115200)

    #reading serial port indefinitely
    while 1:
        print(ser.readline().decode("utf-8", errors='ignore'), end = '')

    #closing port on script exit
    atexit.register(ser.close())
    
