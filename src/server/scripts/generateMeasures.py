import requests
from datetime import date, datetime
from random import randrange
import time
import threading

from requests.api import request

DEV_MODE = False

DATA_SERVICE_URL = 'http://46.101.232.24:8080' if DEV_MODE is False else  "http://localhost:8080"

TERRARIUMS_GET_API = '/data/terrariums'
SESSION_START_API = '/data/terrariums/sessions/start'
ADD_MEASURE_API = '/data/measures'


def getTerrariumsObjects():
    response = requests.get(DATA_SERVICE_URL+TERRARIUMS_GET_API)
    data = response.json()
    return data["data"]


def generateAddRequestFromTerrarium(terrarium):
    now = datetime.now()
    jsonContent = {"Data": []}
    for sensor in terrarium["Sensors"]:
        jsonContent["Data"].append(
            {
                "TerrariumID": terrarium["ID"],
                "SensorID": sensor["ID"],
                "Value": str(20 + randrange(15)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            }
        )
    return jsonContent


def genReqPayloadWithSession(terrarium, session):
    now = datetime.now()
    jsonContent = {"Data": []}
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
            count += 1
            if count > 3:
                break
            jsonPayload = generateAddRequestFromTerrarium(terrarium)
            print("- Sending request - " +
                  datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            requests.post(DATA_SERVICE_URL+ADD_MEASURE_API, json=jsonPayload)
        time.sleep(10)


def simulateLiveUpdateFromId(terrariumID):
    terrariums = getTerrariumsObjects()
    print(terrariums)
    for terrarium in terrariums:
        if terrarium['ID'] == terrariumID:
            response = requests.post(
                DATA_SERVICE_URL+SESSION_START_API, json={'TerrariumID': terrariumID})
            response = response.json()
            print("Init new session : " + response['SessionKey'])
            while True:
                SessionKey = response['SessionKey']
                jsonPayload = genReqPayloadWithSession(terrarium, SessionKey)
                print("- Sending request - " +
                      datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                requests.post(DATA_SERVICE_URL +
                              ADD_MEASURE_API, json=jsonPayload)
                time.sleep(5)


liveUpdateSimulation()
# simulateLiveUpdateFromId("61277e0986ea4a1397b395ac")
