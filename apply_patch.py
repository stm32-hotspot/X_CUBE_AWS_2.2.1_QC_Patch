#******************************************************************************
# * @file           : apply_patch.py
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
import shutil, os

del_dirPath_list = ['../Projects/NUCLEO-H755ZI',
                    '../Projects/STM32WB5MM-DK',
                    '../Projects/B-L4S5I-IOT01A/Applications/BootLoader_KMS',
                    '../Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_demos_osc',
                    '../Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_tests',
                    '../Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_tests_osc',
                   ]

for dirPath in del_dirPath_list:
  try:
      if os.path.exists(dirPath):
        print("Remove: " + dirPath)
        shutil.rmtree(dirPath, ignore_errors=True)
  except OSError as e:
      print(f"Error:{ e.strerror}")

copy_dirPath_list = ['Projects/B-L4S5I-IOT01A/Applications/FLASH_OptionBytes',
                    'STM32_AWS_QuickConnect',
                    ]

for dirPath in copy_dirPath_list:
  try:
      if not os.path.exists('../'+dirPath):
        print("Move: " + dirPath + '  to ../'+dirPath)
        shutil.move(dirPath, '../'+dirPath)
  except OSError as e:
      print(f"Error:{ e.strerror}")

copy_file_list = ['Projects/B-L4S5I-IOT01A/Applications/Cloud/aws_demos/Src/main.c']

for srcFile in copy_file_list:
  try:
      dst_file = os.path.join('../', srcFile)

      if os.path.exists(dst_file):
          print("Remove: " + dst_file)
          shutil.rmtree(dst_file, ignore_errors=True)

      print("Copy file: " + srcFile +" to " + dst_file)
      shutil.copyfile(srcFile, dst_file)
  except OSError as e:
      print(f"Error:{ e.strerror}")
