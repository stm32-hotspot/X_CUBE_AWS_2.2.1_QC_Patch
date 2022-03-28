#******************************************************************************
# * @file           : BOTO3_cleanupThing.py
# * @brief          : Cleans up thing created by quickConnect.py
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
import boto3
import boto3.session
from BOTO3_configFunctions import *

def main():

    # Initialize Boto3 resources
    this_session = start_session()
    iot = this_session.client('iot')
    thing_name = ''
    
    # Get resources
    thing_found = False
    while not thing_found:
        try:
            thing_name = input("Thing name? \n")
            print("Getting:\n")
            cert_arn = iot.list_thing_principals(thingName = thing_name)['principals'][0]
            thing_found = True
        except iot.exceptions.ResourceNotFoundException:
            print("Thing " + thing_name + " not found.\n\n")
    print("\tCertificate Arn - " + cert_arn + "\n")
    cert_id = cert_arn.partition('/')[2]
    print("\tCertificate ID - " + cert_id + "\n")
    policy_name = iot.list_attached_policies(target = cert_arn)['policies'][0]['policyName']
    print("\tPolicy Name - " + policy_name + "\n")

    # Clean thing
    print("Detaching certificate from thing...\n")
    iot.detach_thing_principal(
        thingName = thing_name, 
        principal = cert_arn
    )
    print("Detaching policy from certificate...\n")
    iot.detach_policy(
        policyName = policy_name, 
        target = cert_arn
    )
    print("Deleting thing...\n")
    iot.delete_thing(
        thingName = thing_name
    )
    print("Deleting policy...\n")
    iot.delete_policy(
        policyName = policy_name
    )
    print("Deactivating certificate...\n")
    iot.update_certificate(
        certificateId = cert_id, 
        newStatus = 'INACTIVE'
    )
    print("Revoking certificate...\n")
    iot.update_certificate(
        certificateId = cert_id, 
        newStatus = 'REVOKED'
    )
    print("Deleting certificate...\n")
    iot.delete_certificate(
        certificateId = cert_id
    )
    
    print("Finished.\n")

if __name__ == "__main__":
    main()