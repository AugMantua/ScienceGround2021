import requests
from datetime import date, datetime
from random import randrange
import time
import threading


def getTerrariumsObjects():
    response = requests.get('http://127.0.0.1:8080/data/terrariums/get')
    data = response.json()
    return data["data"]


def generateAddRequest(ID ):
    now = datetime.now()
    jsonContent = {
    "Data" : 
        [
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe33f",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe340",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe340",
                "Value": str(50 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            }
        ]
    }
    return jsonContent

def postRequest(times,ID, delay) :
    for i in range(0,times):
        print("Sending request, terrarium " + ID + "\n")
        requests.post('http://127.0.0.1:8080/data/measures/add',json=generateAddRequest(ID))
        time.sleep(delay)

def dataUpdateCycle():
    x = []
    x.append(threading.Thread(target=postRequest, args=(1000,"611f737c143452662ecbe342",1)))

    for i in range(0,len(x)):
        x[i].start()
        time.sleep(1)


def quickNDirty():
    now = datetime.now()
    t_1 = {
    "Data" : 
        [
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe33f",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe341",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "611f737c143452662ecbe342",
                "SensorID": "611f737c143452662ecbe340",
                "Value": str(50 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            }
        ]
    }
    t_2 = {
    "Data" : 
        [
            {
                "TerrariumID": "6127e68ba74937b1eccce865",
                "SensorID": "6127e68ba74937b1eccce862",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "6127e68ba74937b1eccce865",
                "SensorID": "6127e68ba74937b1eccce863",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "6127e68ba74937b1eccce865",
                "SensorID": "6127e68ba74937b1eccce864",
                "Value": str(50 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            }
        ]
    }
    t_3 = {
    "Data" : 
        [
            {
                "TerrariumID": "6127e6cea74937b1eccce86d",
                "SensorID": "6127e6cea74937b1eccce86a",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "6127e6cea74937b1eccce86d",
                "SensorID": "6127e6cea74937b1eccce86b",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": "6127e6cea74937b1eccce86d",
                "SensorID": "6127e6cea74937b1eccce86c",
                "Value": str(50 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            }
        ]
    }
    print("Sending - ")
    requests.post('http://127.0.0.1:8080/data/measures/add',json=t_1)
    time.sleep(1)
    requests.post('http://127.0.0.1:8080/data/measures/add',json=t_2)
    time.sleep(1)
    requests.post('http://127.0.0.1:8080/data/measures/add',json=t_3)
    time.sleep(1)



def fillData(ID, nrOfMeasures):
    now = datetime.now()
    day = now.day
    month = now.month
    year = now.year

    hour = now.hour
    minute = now.minute
    second = 0

    json = { "Data" : [] }

    for i in range(1,nrOfMeasures):
        minute = minute - 1
        if minute <= 0:
            minute = 59
            hour = hour -1
        if hour <= 0:
            minute = 59
            hour = 23
            day = day - 1
        if day <= 0:
            month = month - 1
            day = 28

        json["Data"].append({
            {
                "TerrariumID": ID,
                "SensorID": "Temp sensor 1",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": ID,
                "SensorID": "Temp sensor 2",
                "Value": str(20 + randrange(2)),
                "Timestamp": now.strftime("%Y-%m-%d %H:%M:%S")
            },
            {
                "TerrariumID": ID,
                "SensorID": "Hum sensor 1",
                "Value": str(50 + randrange(2)),
                "Timestamp":now.strftime("%Y-%m-%d %H:%M:%S")
            }
        })

getTerrariumsObjects()
#for i in range(1,1000):
#    quickNDirty()