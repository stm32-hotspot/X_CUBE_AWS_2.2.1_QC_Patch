#******************************************************************************
# * @file           : quickConnect.py
# * @brief          : Automatically registers new thing to AWS associated with connected STM32.
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
import time
import platform
from BOTO3_configFunctions import *
from ST_configFunctions import *

# 2.0.0 currently
if platform.system() == 'Windows': 
    # EWARM
    #option_bytes = "..\..\Projects\B-L4S5I-IOT01A\Applications\FLASH_OptionBytes\EWARM\X-CUBE-AWS_2.2.0\Exe\X_CUBE_AWS_2.2.0_FLASH_OptionBytes.bin"
    #stsafe       = "..\..\Projects\B-L4S5I-IOT01A\Applications\BootLoader_STSAFE\STSAFE_Provisioning\EWARM\Provisioning.bin"
    #aws_config   = "..\..\Projects\B-L4S5I-IOT01A\Applications\EEPROM\AWS_Config\EWARM\AWS_Config\Exe\AWS_Config.bin"
    #main_app     = "..\..\Projects\B-L4S5I-IOT01A\Applications\Cloud\\aws_demos\EWARM\PostBuild\SBSFU_B-L4S5I-IOT01_aws_demos.bin"

    # CubeIDE
    option_bytes = "..\..\Projects\B-L4S5I-IOT01A\Applications\FLASH_OptionBytes\EWARM\X-CUBE-AWS_2.2.0\Exe\X_CUBE_AWS_2.2.0_FLASH_OptionBytes.bin"
    stsafe       = "..\..\Projects\B-L4S5I-IOT01A\Applications\BootLoader_STSAFE\STSAFE_Provisioning\STM32CubeIDE\B-L4S5I-IOT01_STSAFE_Provisioning\Debug\B-L4S5I-IOT01_STSAFE_Provisioning.bin"
    aws_config   = "..\..\Projects\B-L4S5I-IOT01A\Applications\EEPROM\AWS_Config\EWARM\AWS_Config\Exe\AWS_Config.bin"
    main_app     = "..\..\Projects\B-L4S5I-IOT01A\Applications\Cloud\\aws_demos\STM32CubeIDE\PostBuild\SBSFU_B-L4S5I-IOT01_aws_demos.bin"
#Mac or linux
else: 
    # EWARM
    #option_bytes = "../../Projects/B-L4S5I-IOT01A/Applications/FLASH_OptionBytes/EWARM/X-CUBE-AWS_2.2.0/Exe/X_CUBE_AWS_2.2.0_FLASH_OptionBytes.bin"
    #stsafe       = "../../Projects/B-L4S5I-IOT01A/Applications/BootLoader_STSAFE/STSAFE_Provisioning/EWARM/Provisioning.bin"
    #aws_config   = "../../Projects/B-L4S5I-IOT01A/Applications/EEPROM/AWS_Config/EWARM/AWS_Config/Exe/AWS_Config.bin"
    #main_app     = "../../Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_demos/EWARM/PostBuild/SBSFU_B-L4S5I-IOT01_aws_demos.bin"

    # Cube IDE
    option_bytes = "../../Projects/B-L4S5I-IOT01A/Applications/FLASH_OptionBytes/EWARM/X-CUBE-AWS_2.2.0/Exe/X_CUBE_AWS_2.2.0_FLASH_OptionBytes.bin"
    stsafe       = "../../Projects/B-L4S5I-IOT01A/Applications/BootLoader_STSAFE/STSAFE_Provisioning/STM32CubeIDE/B-L4S5I-IOT01_STSAFE_Provisioning/Debug/B-L4S5I-IOT01_STSAFE_Provisioning.bin"
    aws_config   = "../../Projects/B-L4S5I-IOT01A/Applications/EEPROM/AWS_Config/EWARM/AWS_Config/Exe/AWS_Config.bin"
    main_app     = "../../Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_demos/STM32CubeIDE/PostBuild/SBSFU_B-L4S5I-IOT01_aws_demos.bin"



def main():

    # Initialize session resources 
    this_session = start_session()
    collect_endpoint(this_session)
    session_os = platform.system()
    print("Operating system is " + session_os)
    COM = get_com()
    print("Serial port is " + COM)
    USBPATH = find_path(session_os)
    print("USB mounted to " + USBPATH)

    # Set option bytes
    print("Setting Option Bytes \n")
    flash_board(option_bytes, USBPATH, COM)

    # Flashing provisioning
    print("Provisioning STSafe\n")
    flash_board(stsafe, USBPATH, COM)
    time.sleep(5)

    # Configure EEPROM
    #print("Updating EEPROM \n")
    #flash_board(aws_config, USBPATH, COM)
    #update_eeprom(COM)

    # Flash application binary
    print("Flashing main application binary \n")
    flash_board(main_app, USBPATH, COM)

    # Get cert
    print("Retrieving Certificate from STM32 \n")
    get_cert_from_stm32(COM)
    prep_cert(retrieve_config("ThingName"))

    # Register thing to AWS
    print("Registering thing on AWS \n")
    register_device(this_session)
    time.sleep(5)

    # Display serial communication
    serial_reader(COM)

if __name__ == "__main__":
    main()
