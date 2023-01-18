#!/bin/python

from pymodbus.client.sync import ModbusTcpClient
from pymodbus.client.common import ModbusClientMixin
from pymodbus.constants import Defaults
from pyfirmata import Arduino
import serial

import time

Defaults.RetryOnEmpty = True

#-----------------------------variable definition for receiving data
tur_rpm = 0.0      #Turbine rpm, address 1
max_rpm = 0.0      #Maximum rpm of all turbines
max_rpm0 = 0.0     #Maximum rpm for before time step
tur_vol = 0.0      #DC voltage from turbine, address 2
max_tur_vol = 0.0  #Maximum DC voltage from all turbines
inv_pwr = 0.0      #inverter power, address 24
dump_pwr = 0.0     #dumploaded power, address 5
total_pwr = 0.0     #total power = inverter power + dumploaded power
max_pwr = 0.0       #maximum total power
cont_in_vol = 0.0   #controller operating input voltage, address 3
windspeed = 0.0     #wind speed, address 0
fpin = 13           #실린더 후진 folding arduino pin
upin = 12           #실린더 전진 unfolding arduino pin
ycwpin = 11         #CW direction yawing arduino pin
yccwpin = 10        #CCW direction yawing arduino pin
motor_start = 9     #motor operating arduino pin

#-------------------------------arduino initialize
port1='COM12' #control arduino port
baudrate=9600
board1=Arduino(port1)


fpin=board1.get_pin('d:13:o') # folding pin setting
upin=board1.get_pin('d:12:o') # unfolding pin setting
ycwpin=board1.get_pin('d:11:o') # unfolding pin setting
yccwpin=board1.get_pin('d:10:o') # unfolding pin setting
motor_start=board1.get_pin('d:9:o') # unfolding pin setting

#pin intitialize for arduino
fpin.write(0)
upin.write(0)
ycwpin.write(0)
yccwpin.write(0)
motor_start.write(0)

# current mode information
op_mode = "manual"   #op_mode initialize
print("The Current mode is ", op_mode," operating.")

# Select operating mode for tailwing contro
while True:  # main while block start
    op_mode = str(input("Select operating mode: manual:m, auto:a, e-stop:e "))
 #===== select manual mode and start (only contact arduino)===================
    if op_mode == "m" or op_mode =="M" or op_mode == "manual":
        op_mode = "manual"
        print("The Current mode is ", op_mode," operating.")
 # manual mode control start,
        while True:
            try:
                print("If you want to stop manual operating mode, press Ctrl+C")
                mode_num = str(input("Select motion :0-Stop, 1-Folding, 2-Unfolding : "))
                count = 0
                if mode_num == "1":
                    fpin.write(1)
                    upin.write(0)
                    while count < 100:
                        count = count + 1
                        time.sleep(0.1)        # loop time delay
                        print("Loop count = " + str(count))
                elif mode_num == "2":
                    fpin.write(0)
                    upin.write(1)
                    while count < 100:
                        count = count + 1
                        time.sleep(0.1)
                        print("Loop count = " + str(count))
                elif mode_num == "0":
                    fpin.write(0)
                    upin.write(0)
                    print("Tail wing Stop ")
                elif mode_num != "1" and mode_num != "2" and mode_num != "0":
                    print("Input is wrong, seletc again ")
                    fpin.write(0)
                    upin.write(0)
            except KeyboardInterrupt:
                fpin.write(0)
                upin.write(0)
                print("Manual operating mode stopped")
            break
 # manual mode control end, and return to main while block
        continue
 # ============ manual mode end =========================================

 # ====== select auto mode and start (contact both ardoino anc TCP)======
    elif op_mode == "a" or op_mode == "auto":
        op_mode ="auto"
        print("The Current mode is ", op_mode," operating.")
        print("If you want to stop auto operating mode, press Ctrl+C")

#----------------------------------------Data receive from TCP
        while True:
            try:
# connection to controller
                client1 = ModbusTcpClient(host="192.168.0.112", port=502)
                print(client1.connect())
# read data from controller
                result = client1.read_input_registers(address=0, count=50, unit=i) #read data set

                tur_rpm = float(result.registers[1]) * 3.8  #read turbine rpm
                tur_vol = float(result.registers[2])   #read turbine DC voltate
                cont_in_vol = float(result.registers[3]) #read controller operating power voltate
                dump_pwr = float(result.registers[5]) #read dumploaded power
                inv_pwr = float(result.registers[24]) #read inverting power
                print("turbine ", i, "rpm= ", tur_rpm)

                if max_rpm<=tur_rpm:   # maximum rpm check
                    max_rpm = tur_rpm
                else:
                    max_rpm = max_rpm

                if max_tur_vol<=tur_vol:  # maximum turbine voltage check
                    max_tur_vol = tur_vol
                else:
                    max_tur_vol = max_tur_vol

                total_pwr = dump_pwr + inv_pwr  # maximum power check
                if max_pwr<=total_pwr:
                    max_pwr = total_pwr
                else:
                    max_pwr = max_pwr

                client1.close()

# slop calculation
                if max_rpm > 0:
                    slop_rpm = (max_rpm - max_rpm0)/max_rpm
                    max_rpm0 = max_rpm
                else:
                    slop_rpm = 0.0

# connection to weather tower
                client2 = ModbusTcpClient(host="192.168.0.140", port=502)
                print(client2.connect())
# read wind speed data
                result = client2.read_holding_registers(address=0, count=10, unit=1) #read data set
                windspeed = float(result.registers[0]) #read wind speed
                print("Wind speed = ", windspeed, "m/s")

                client2.close()
#  auto mode control start, select mode
                # reference control rpm = 80rpm,
                # rpm go-up condition : below 10% of ref, 72rpm->folding,
                # rpm go-down condition : upper 5% of ref, 84rpm->unfolding,
                goup_rpm = 72
                godn_rpm = 84
                mode_num = "0" # initialize mode_num for stop

            # motion define :0-Stop, 1-Folding, 2-Unfolding
                if max_rpm < goup_rpm :
                    mode_num = "0"
                elif max_rpm >= goup_rpm and max_rpm < godn_rpm :
                    if slop_rpm >= -0.05 and slop_rpm <0.05 :
                        mode_num = "0"
                    elif slop_rpm >= 0.05 and slop_rpm <0.3 :
                        mode_num = "1"
                        optime = 100
                    elif slop_rpm > 0.3 :
                        mode_num = "1"
                        optime = 300
                    elif slop_rpm >= -0.3 and slop_rpm <-0.05 :
                        mode_num = "2"
                        optime = 100
                    elif slop_rpm < -0.3 :
                        mode_num = "2"
                        optime = 300
                elif max_rpm >= godn_rpm and max_rpm <120 :
                    if slop_rpm >= -0.05 and slop_rpm <0.05 :
                        mode_num = "0"
                    elif slop_rpm >= 0.05 and slop_rpm <0.3 :
                        mode_num = "1"
                        optime = 100
                    elif slop_rpm > 0.3 :
                        mode_num = "1"
                        optime = 300
                    elif slop_rpm >= -0.3 and slop_rpm <-0.05 :
                        mode_num = "2"
                        optime = 50
                    elif slop_rpm < -0.3 :
                        mode_num = "2"
                        optime = 150

                count = 0 # initialize time loop count for activate the actuator
            #select folding/unfolding/Stop
                if mode_num == "1":
                    fpin.write(1)
                    upin.write(0)
                    while count < optime:
                        count = count + 1
                        time.sleep(0.1)        # loop time delay
                        print("Loop count = " + str(count))
                elif mode_num == "2":
                    fpin.write(0)
                    upin.write(1)
                    while count < optime:
                        count = count + 1
                        time.sleep(0.1)
                        print("Loop count = " + str(count))
                elif mode_num == "0":
                    fpin.write(0)
                    upin.write(0)
                    print("Tail wing Stop ")

                fpin.write(0) # actuator stop
                upin.write(0) # actuator stop

                continue  # comback auto mode while loop
            except KeyboardInterrupt:
                fpin.write(0) # actuator stop
                upin.write(0) # actuator stop
                print("Auto operating mode stopped")
                break

 # auto mode control end, and return to main while block
        continue
