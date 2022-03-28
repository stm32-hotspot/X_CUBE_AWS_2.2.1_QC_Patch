#******************************************************************************
# * @file           : BOTO3_configFunction.py
# * @brief          : Contains support functions for quickConnect.py related to boto3.
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
import boto3.session
import json
import os

config_location = "Config.txt"

# Returns a sessions with the authentication details from ./Confog.txt.
def start_session():
    access_key = retrieve_config("Access_Key")
    secret_key =  retrieve_config("Secret_Key")
    region = retrieve_config("Region")
    return boto3.session.Session(
        aws_access_key_id = access_key,
        aws_secret_access_key = secret_key,
        region_name = region
    )

# Saves configuration details in the file Config.txt
# Parameters:
#   config_name: String. The title of the configuration data. (e.g. "Profile", "Endpoint")
#   config: String. The matching data. (e.g. for the Profile example, "QuickConnect")
def save_config(config_name, config):
    with open(config_location, "r") as f:
        lines = f.readlines()
        with open(config_location, "w") as f:
          for line in lines:
            if line.startswith(config_name) == False:
              f.write(line.strip('\n') + '\n')
        
    out = open(config_location, "a")
    out.write(config_name + ":\t" + config)
    out.close()

# Retrieves configuration details in the file Config.txt
# Parameters:
#   config_name: String. The title of the requested configuration data.
#
# Returns: String. The data corresponding to the config_name. If matching data not found, returns " ERR ".
def retrieve_config(config_name):
    with open(config_location, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith(config_name):
                return line.split()[-1]
    return config_name + " ERR "
    
# Retrieves thing's certificate from /Cert/ folder
# Parameters:
#   thing: String. Name of the thing related to the cert.
#  
# Returns: String. Certificate.
def retrieve_cert(thing_name):
    cert = ""
    with open("./Certs/" + thing_name + "_cert.pem", 'r') as f:
        lines = f.readlines()
        for line in lines:
            cert += line
    return cert

# Builds the policy for the thing as a json object
#
# Parameters:
#   thingArn: String. The Amazon Resource Name (Arn) for the thing. (eg-- arn:aws:iot:us-east-2:505204505951:thing/C201300004)
#
# Returns: json.dump that describes the policy
def build_thing_policy(thingArn):
    
    acctArn = thingArn.rpartition(':')[0]
    acctID = acctArn.rpartition(':')[2]

    dictionary = {
    "Version": "2012-10-17",
    "Statement": [
        {
        "Effect": "Allow",
        "Action": "iot:Connect",
        "Resource": acctArn + ':client/${iot:Connection.Thing.ThingName}'
        },
        {
        "Effect": "Allow",
        "Action": "iot:Subscribe",
        "Resource": acctArn + ":*"
        },
        {
        "Effect": "Allow",
        "Action": "iot:Receive",
        "Resource": acctArn + ":*"
        },
        {
        "Effect": "Allow",
        "Action": "iot:Publish",
        "Resource": acctArn + ":*"
        }
    ]
    }

    # Serializing json  
    json_object = json.dumps(dictionary, indent=4)

    # Writing to sample.json 
    if not os.path.exists("./Policies/"):
        os.mkdir("./Policies/")

    with open("Policies/" + acctID + "_STSafeThingPolicy.json", "w") as outfile: 
        outfile.write(json_object) 
    
    return json_object

# Collects AWS endpoint and saves it to ./Config.txt
# Parameters:
#   this_session: BOTO3 Session object. Session for desired endpoint.
def collect_endpoint(this_session):

    # Initialize Boto3 resources
    iot = this_session.client('iot')
    endpoint = iot.describe_endpoint()['endpointAddress']

    # Save information to Config.txt
    save_config("Endpoint", endpoint)

# Registers device on AWS
def register_device(this_session):
    
    # Retrieve config data
    prof_name = retrieve_config("Profile")
    thing_name = retrieve_config("ThingName")
    cert_pem = retrieve_cert(thing_name)

    # Initialize Boto3 resources
    iot = this_session.client('iot')
    
    # Set local policy name
    policy_name = thing_name + "_Policy"

    # Create Thing dict
    print("Creating thing named %s...\n" % thing_name)
    thing = iot.create_thing(
        thingName = thing_name
    )

    # Create Cert dict
    print("Creating certificate...\n")
    cert = iot.register_certificate_without_ca(
        certificatePem = cert_pem, 
        status = 'ACTIVE'
    )

    # Build Policy
    print("Building policy...\n")
    policy_string = build_thing_policy(thing['thingArn'])
    policy = iot.create_policy(
        policyName = policy_name, 
        policyDocument = policy_string
    )

    # Attach cert and policy to thing
    print("Attaching policy to certificate...\n")
    iot.attach_policy(
        policyName = policy_name,
        target = cert['certificateArn']
    )
    print("Attaching certificate to thing...\n")
    iot.attach_thing_principal(
        thingName = thing['thingName'],
        principal = cert['certificateArn']
    )

    # Save Certificate Arn
    #save_config("Certificate", cert['certificateId'])
    #save_config("Policy", '\t' + policy['policyName'])