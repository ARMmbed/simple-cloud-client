#!/usr/bin/env python

# graceful exit
import sys

# parse JSON
import json
from pprint import pprint

# mbed connector support
import mbed_connector_api

# sleep while waiting for response from connector
import time

# decode response
import base64

# load data from file with named passed as first argument
def parseInput():
    try:
        file_name = sys.argv[1]
        with open(file_name, "rb") as data_file:
            data = data_file.read()

    except IndexError:
        print "Usage: %s <file to post>" % sys.argv[0]
        sys.exit()

    except IOError:
        print "File %s not found." % file_name
        sys.exit()

    return data

# load configuration from JSON file
def loadConfiguration():
    try:
        with open('uc-config.json') as config_file:
            config = json.load(config_file)

    except IOError:
        print "uc-config.json file not found."
        print "Create JSON file with fields:"
        print "{"
        print "    \"access-key\": <key>,"
        print "    \"url\": <LWM2M server address and protocol>,"
        print "    \"port\": <remote port>,"
        print "    \"resource\": <post resource>,"
        print "    \"devices\": ["
        print "        <device name>,"
        print "        ..."
        print "    ]"
        print "}"
        sys.exit()

    return (config['access-key'], config['url'], config['port'], config['resource'], config['devices'])

def notification(message):
    path = message['notifications'][0]['path']
    endpoint = message['notifications'][0]['ep']
    payload64 = message['notifications'][0]['payload'][:2] + "=="
    payload = base64.b64decode(payload64)

    if path == "/5/0/3":
        if payload == "0":
            print "%s : status : idle" % endpoint
        elif payload == "1":
            print "%s : status : downloading" % endpoint
        elif payload == "2":
            print "%s : status : downloading done" % endpoint
        elif payload == "3":
            print "%s : status : updating" % endpoint
        else:
            print "%s : status : state error" % endpoint
    elif path == "/5/0/5":
        if payload == "0":
            print "%s : result : initial" % endpoint
        elif payload == "1":
            print "%s : result : success" % endpoint
        elif payload == "2":
            print "%s : result : error storage" % endpoint
        elif payload == "3":
            print "%s : result : error memory" % endpoint
        elif payload == "4":
            print "%s : result : error connection" % endpoint
        elif payload == "5":
            print "%s : result : error crc" % endpoint
        elif payload == "6":
            print "%s : result : error type" % endpoint
        elif payload == "7":
            print "%s : result : error uri" % endpoint
        elif payload == "8":
            print "%s : result : error update" % endpoint
        else:
            print "%s : result : state error" % endpoint

def connect(apikey, url, port):
    # connect to connector
    client = mbed_connector_api.connector(apikey, url, port)

    # start long polling for receiving results
    client.startLongPolling()

    # set notification handler
    client.setHandler('notifications', notification)

    return client

def subscribe(client, resource, devices):

    # for each endpoint listed in uc-config.json
    for endpoint in devices:

        result = client.putResourceSubscription(endpoint, resource)

        # reset timeout
        timeout = 0

        # wait for and display result
        while ((not result.isDone()) and (timeout < 120)):
            time.sleep(1)
            timeout += 1

        if timeout >= 120:
            print "%s : subscription timeout (%s)" % (endpoint, resource)
        elif result.error:
            print "%s : %s (%s)" % (endpoint, result.error.error, resource)
            break
        else:
            print "%s : subscription success (%s)" % (endpoint, resource)
            break

# send data to endpoints
def postToDevices(client, resource, devices, data):

    # for each endpoint listed in uc-config.json
    for endpoint in devices:

        for retry in range(0, 3):
            # reset timeout
            timeout = 0

            # post data to resource on endpoint
            result = client.postResource(endpoint, resource, data)

            # wait for and display result
            while ((not result.isDone()) and (timeout < 120)):
                time.sleep(1)
                timeout += 1

            if timeout >= 120:
                print "%s : send manifest timeout" % endpoint
            elif result.error:
                print "%s : %s" % (endpoint, result.error.error)
                break
            else:
                print "%s : send manifest success" % endpoint
                break

# Module test harness for standalone testing.
if __name__ == "__main__":
    # load data from file
    data = parseInput()

    # load configuration from file
    (apikey, url, port, resource, devices) = loadConfiguration()

    # connect using API key
    mbed_client = connect(apikey, url, port)

    # pretty print output
    print " Endpoint                              Status"
    print "----------------------------------------" + \
          "----------------------------------------"

    # subscribe to notifications
    subscribe(mbed_client, "/5/0/3", devices)
    subscribe(mbed_client, "/5/0/5", devices)

    # send data using post
    postToDevices(mbed_client, resource, devices, data)

    while 1:
        time.sleep(1)
