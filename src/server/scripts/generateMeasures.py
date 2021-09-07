import requests
from datetime import date, datetime
from random import randrange
import time
import threading

from requests.api import request


def getTerrariumsObjects():
    response = requests.get('http://127.0.0.1:8080/data/terrariums/get')
    data = response.json()
    return data["data"]


def generateAddRequestFromTerrarium(terrarium):
    now = datetime.now()
    jsonContent = {"Data" : []}
    for sensor in terrarium["Sensors"]:
        jsonContent["Data"].append(
                    {
                        "TerrariumID": terrarium["ID"],
                        "SensorID": sensor["ID"],
                        "Value": str(20 + randrange(2)),
                        "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
                    }
            )
    return jsonContent

def genReqPayloadWithSession(terrarium,session):
    now = datetime.now()
    jsonContent = {"Data" : []}
    for sensor in terrarium["Sensors"]:
        jsonContent["Data"].append(
                    {
                        "TerrariumID": terrarium["ID"],
                        "SensorID": sensor["ID"],
                        "Value": str(20 + randrange(2)),
                        "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S"),
                        "SessionKey": session
                    }
            )
    return jsonContent

def liveUpdateSimulation():
    terrariums = getTerrariumsObjects()
    while True:
        count = 0
        for terrarium in terrariums:
                count+=1
                if count > 3 :
                    break
                jsonPayload = generateAddRequestFromTerrarium(terrarium)
                print("- Sending request - " + datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                requests.post('http://127.0.0.1:8080/data/measures/add',json=jsonPayload)
        time.sleep(5)


def simulateLiveUpdateFromId(terrariumID):
    terrariums = getTerrariumsObjects()
    for terrarium in terrariums:
        if terrarium['ID'] == terrariumID:
            response = requests.post('http://127.0.0.1:8080/data/terrariums/sessions/start',json={'TerrariumID':terrariumID})
            response = response.json()
            print("Init new session : " + response['SessionKey'])
            while True:
                SessionKey = response['SessionKey']
                jsonPayload = genReqPayloadWithSession(terrarium,SessionKey)
                print("- Sending request - " + datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                requests.post('http://127.0.0.1:8080/data/measures/add',json=jsonPayload)


#liveUpdateSimulation()
simulateLiveUpdateFromId("6133814483343737296c7c8c")

