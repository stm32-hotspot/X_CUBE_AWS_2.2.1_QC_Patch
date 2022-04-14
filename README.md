# X_CUBE_AWS_2.2.1 Quick Connect Patch
Prepare [X-CUBE-AWS 2.2.1](https://www.st.com/en/embedded-software/x-cube-aws.html) For Quick Connect using the [B-L4S5I-IOT01A](https://www.st.com/en/evaluation-tools/b-l4s5i-iot01a.html) board

# Table of Contents

1. [What is X_CUBE_AWS_2.2.1 Quick Connect?](#1-what-is-x_cube_aws_221-quick-connect)
2. [What is X_CUBE_AWS_2.2.1 Quick Connect Patch?](#2-what-is-x_cube_aws_221-quick-connect-patch)
3. [Prerequisite](#3-prerequisite)
4. [Get the firmware packs](#4-get-the-firmware-packs)
5. [Apply the patch](#5-apply-the-patch)
6. [Open the projects](#6-open-the-projects)
7. [Configure and build the projects](#7-configure-and-build-the-projects)
8. [Run the Quick Connect script](#8-run-the-quick-connect-script)
9. [Check your device in AWS IoT Core](#9-check-your-device-in-aws-iot-core)
10. [Run the shadow demo ](#10-run-the-shadow-demo)
11. [Monitoring MQTT messages on the cloud](#11-monitoring-mqtt-messages-on-the-cloud)
12. [Cleaning up](#12-cleaning-up)

# 1. What is X_CUBE_AWS_2.2.1 Quick Connect?
**X_CUBE_AWS_2.2.1 Quick Connect** is a script that automates most of the steps required to connect [B-L4S5I-IOT01A](https://www.st.com/en/evaluation-tools/b-l4s5i-iot01a.html) board and [X-CUBE-AWS 2.2.1](https://www.st.com/en/embedded-software/x-cube-aws.html) to AWS making it easy and takes less time.

The script will:

- Flash the firmwares required by X_CUBE_AWS_2.2.1 in the correct order
	- **FLASH_OptionBytes**  : Sets the STM32L4+ option bytes as required by X_CUBE_AWS_2.2.1
	- **STSAFE_Provisioning**: Provision STSAFE
	- **aws_demos**          : Main application (a combined binary file that contains SBSFU and the signed main application that connects to AWS)
	
- Retreive the device certificate from STSAFE
	
- Retreive the the thing name
	
- Register the device with AWS:
	- Create a Thing
	- Upload the certificate
	- Attach certificate to thing
	- Attach policy to the certificate

# 2. What is X_CUBE_AWS_2.2.1 Quick Connect Patch?
- The **X_CUBE_AWS_2.2.1 Quick Connect Patch** will remove the projects that are not needed by this Quick Connect. This to remove the confusion on which projects you need to import and build with STM32CubeIDE
- Copy the Quick Connect script
- Change the aws_demos **main.c** file to be compatible with X_CUBE_AWS_2.2.1 Quick Connect
- Update the project to support **[STM32CubeIDE](https://www.st.com/stm32cubeide)** rev 1.9.0


# 3. Prerequisite
**[STM32CubeIDE](https://www.st.com/stm32cubeide)** rev 1.9.0

**[STM32CubeProgrammer](https://www.st.com/stm32cubeprog)**

**[Python](https://www.python.org/).**

- NOTE: When installing Python make sure to install pip by selecting pip under Optional Features
-	NOTE: When installing Python make sure to select 'Add Python to the Path' during installation.
- You need to install pyserial and boto3

```
pip install pyserial
pip install boto3
```

**[Create an IAM user in your AWS account](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_create.html)**
- You need the **Access_Key** and **Secret_Key**. ***(Keep them safe and don't share)***
- Below is the recommanded policy for your IAM user. Replace **REGION_NAME** with the desired region for the device (example: us-east-2).

```
{
    	"Version": "2012-10-17",
    	"Statement": [
        	{
            	"Sid": "VisualEditor0",
        	    "Effect": "Allow",
	            "Action": [
                	"iot:DetachThingPrincipal",
                	"iot:CreateThing",
                	"iot:DeleteThing",
                	"iot:AttachThingPrincipal",
                	"iot:DetachPolicy",
                	"iot:DeleteCertificate",
                	"iot:AttachPolicy",
                	"iot:RegisterCertificateWithoutCA",
                	"iot:CreatePolicy",
                	"iot:ListAttachedPolicies",
                	"iot:DescribeEndpoint",
                	"iot:DeletePolicy",
                	"iot:ListThingPrincipals",
                	"iot:UpdateCertificate"
            	],
            	"Resource": "*",
            	"Condition": {
        	        "StringEquals": {
	                    "aws:RequestedRegion": "REGION_NAME"
                	}
        	    }
	        }
	    ]
	}
 ```
 
 
**Wi-Fi network**
- You need 2.4 GHz

**[B-L4S5I-IOT01A](https://www.st.com/en/evaluation-tools/b-l4s5i-iot01a.html) board**

***NOTE: This was tested on Windows 10 only***

# 4. Get the firmware packs
- Download and extreact [X-CUBE-AWS 2.2.1](https://www.st.com/en/embedded-software/x-cube-aws.html)
- Navigate to **STM32CubeExpansion_Cloud_AWS_V2.2.1** directory
- Clone the repo in your **STM32CubeExpansion_Cloud_AWS_V2.2.1** directory

```
git clone https://github.com/stm32-hotspot/X_CUBE_AWS_2.2.1_QC_Patch.git
```

![X_CUBE_AWS_2 2 1_QC_Patch](https://user-images.githubusercontent.com/41168224/160009472-25c10564-b7f2-4365-a5bb-6e4496d480b2.png)

# 5. Apply the patch
- Navigate to **X_CUBE_AWS_2.2.1_QC_Patch**

![QC_Patch](https://user-images.githubusercontent.com/41168224/160170716-2efcb0b5-c7b8-4a0f-bb35-239ad5e1d7df.png)

- Apply the patch. You can open a PowerShell Window by going to ***File->Open Windows PowerShell***

 ![PowerShell](https://user-images.githubusercontent.com/41168224/160171023-a3df1002-f406-406d-9960-aee7eeff866c.png)

- Type the following command

```
python .\apply_patch.py
```

<img width="1025" alt="run_apply_patch" src="https://user-images.githubusercontent.com/41168224/160037873-c038bc95-ef63-41e6-a002-916cb34d5220.png">

# 6. Open the projects
- Open [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) new work space

- Click **Browse...**

<img width="432" alt="cube_ide_browse" src="https://user-images.githubusercontent.com/41168224/160032757-5255098b-a126-4717-bcc7-72d5a55721bc.png">

- Create new directory

 <img width="426" alt="cube_ide_crate_dir" src="https://user-images.githubusercontent.com/41168224/160032782-9ed1182b-4220-4f53-885c-8f79e2449535.png">

- Select the new directory
- Click **Launch**

- Close the **Information Center**

![information_center](https://user-images.githubusercontent.com/41168224/160014723-d65da8b6-dc9b-405d-8c0b-19bdd112a4a2.png)

- Click **Import projects** 

 ![import_project_1](https://user-images.githubusercontent.com/41168224/160014837-5a3b7527-eccc-4b69-a283-079a95c8cd7b.png)

- Import the projects present in the **\Projects\B-L4S5I-IOT01A\Applications** directory

<img width="919" alt="cube_ide_select_folder" src="https://user-images.githubusercontent.com/41168224/160032821-99b75e95-b4dd-4706-9290-63b86987e14e.png">

- Click **Finish**
![import_project_2](https://user-images.githubusercontent.com/41168224/160015124-eb12137d-14f8-4fa2-9691-0bda62cbd4a1.png)

# 7. Configure and build the projects

- Open **aws_clientcredential.h**. This is how you find it
 
![open_aws_clientcredential](https://user-images.githubusercontent.com/41168224/160010829-7975ba75-b4e6-43e7-bc99-d87f38441925.png)


![open_declaration](https://user-images.githubusercontent.com/41168224/160021076-87920f35-59e8-4706-abec-53f098ff4094.png)

- Get your endpoint from AWS IoT Core. You find it under ***Settings***

![endpoint](https://user-images.githubusercontent.com/41168224/160027913-4240a649-f2e1-4e44-bcee-15feb07700a3.png)

- Edit **aws_clientcredential.h**
```
#define clientcredentialMQTT_BROKER_ENDPOINT         "xzy-ats.iot.us-east-2.amazonaws.com"
#define clientcredentialIOT_THING_NAME               "IOT_THING_NAME"
#define clientcredentialWIFI_SSID                    "MySSID"
#define clientcredentialWIFI_PASSWORD                "MyPSWD"
```

![aws_clientcredential_2](https://user-images.githubusercontent.com/41168224/160016188-eac884e8-1d48-4c34-8da7-a0d82b747abf.png)


- Save your changes
- Build the projects in the following order:
 
> 1- STSAFE_Provisioning
> 
> 2- 2_Images_SECoreBin
> 
> 3- 2_Images_SBSFU
> 
> 4- aws_demos

# 8. Run the Quick Connect script
- Switch to ***STM32CubeExpansion_Cloud_AWS_V2.2.1\STM32_AWS_QuickConnect\Scripts*** directory

![STM32_AWS_QuickConnect](https://user-images.githubusercontent.com/41168224/160022982-bb8a70a8-86e4-4ef6-947a-cc4e9b866fa6.png)

- Edit **Config.txt**

![config_txt](https://user-images.githubusercontent.com/41168224/160041842-39f315db-0b87-4ed5-9c55-7ae04a644144.png)

![config](https://user-images.githubusercontent.com/41168224/160042184-c9af15c8-585a-47e0-84c1-0a0465a1ad52.png)

***NOTE: The Wi-Fi SSID and PASSWORD are not used in this version of Quick Connect***

***NOTE: Region should be the same as in the enpoint (us-east-2) as in the example below***

```
#define clientcredentialMQTT_BROKER_ENDPOINT         "xzy-ats.iot.us-east-2.amazonaws.com"
```

- Make sure your board is connected to your computer over USB. Make sure to use the ST-Link USB port

- Open a new PowerShell window

![PowerShell_2](https://user-images.githubusercontent.com/41168224/160171988-b6150c12-f37f-47bd-9205-b57efb609cdf.png)

- Execute the **Quick Connect** script

```
python .\STM32_AWS_QuickConnect.py
```

<img width="855" alt="run_qc" src="https://user-images.githubusercontent.com/41168224/160037963-737b9363-d5b3-4b57-b0ba-78dae16cd65a.png">

# 9. Check your device in AWS IoT Core

![check_device_iot_core](https://user-images.githubusercontent.com/41168224/160038221-6b8f3d7b-9d2a-45e2-b265-21ea7167b657.png)

# 10. Run the shadow demo
by default, X-CUBE-AWS comes with the OTA demo enabled. Here is how you can build a different demo. The SHADOW DEMO is used as an example.

 Open **aws_demo_config.h**

![aws_demo_config](https://user-images.githubusercontent.com/41168224/163469528-f8fcf404-2835-48e0-be4f-601898250002.png)

- change the definition on line 45 from **#define CONFIG_OTA_UPDATE_DEMO_ENABLED** to **#define CONFIG_SHADOW_DEMO_ENABLED**

![CONFIG_SHADOW_DEMO_ENABLED](https://user-images.githubusercontent.com/41168224/163469236-8ab25a48-eb2c-4f26-93f0-493f658ae484.png)


- Build the project

![build_aws_demos](https://user-images.githubusercontent.com/41168224/163470866-cd2fa52a-df3b-4f71-89bf-1a7562f7e456.png)


- Flash the board with the **SBSFU_B-L4S5I-IOT01_aws_demos.bin** binary located under **.\STM32CubeExpansion_Cloud_AWS_V2.2.1\Projects\B-L4S5I-IOT01A\Applications\Cloud\aws_demos\STM32CubeIDE\PostBuild**

![SBSFU_B-L4S5I-IOT01_aws_demos](https://user-images.githubusercontent.com/41168224/163470378-71d8fd4a-829c-41dc-833f-3bd11dc21447.png)

# 11. Monitoring MQTT messages on the cloud

To subscribe to the MQTT topic with the AWS IoT MQTT client

- Sign in to the AWS IoT [console](https://console.aws.amazon.com/iotv2/).

- In the navigation pane, choose Test, then choose MQTT test client to open the MQTT client.

- In Subscription topic, enter $aws/things/***<IOT_THING_NAME>***/shadow/update/delta, and then choose Subscribe to topic.

![SHADOW_TOPIC](https://user-images.githubusercontent.com/41168224/163480616-b8b508e2-75b7-456d-bf62-d8fe05d30146.png)


# 12. Cleaning up

- STM32_AWS_CleanupThing.py:: Removes thing created by STM32_AWS_QuickConnect.py and associated certificate and policy.

- Navigate to ..\STM32_AWS_QuickConnect\Scripts\ and run **STM32_AWS_QuickConnect.py**.

- When prompted, enter the **IOT_THING_NAME** of the desired board.

- Monitor the command window to verify the process finishes.
