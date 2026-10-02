#!/usr/bin/python3

from urllib.request import urlopen
import json
from time import sleep

DUMP1090DATAURL = "http://localhost:8080/data/aircraft.json"

class FlightData():
    def __init__(self, data_url = DUMP1090DATAURL):

        self.data_url = data_url

        self.refresh()

    def refresh(self):
        #open the data url
        self.req = urlopen(self.data_url)

        #read data from the url
        self.raw_data = self.req.read()

        #encode the data
        encoding = self.req.headers.get_content_charset('utf8')

        #load in the json
        self.json_data = json.loads(self.raw_data.decode(encoding))

        self.aircraft = AirCraftData.parse_flightdata_json(self.json_data)

class AirCraftData():
    def __init__(self,
                 hex,
                 flight,
                 #track,
                 squawk,
                 lat,
                 lon,
                 #validposition,
                 alt_baro,
                 #geom_rate,
                 #validtrack,
                 gs,
                 #mlat,
                 messages,
                 seen):
        
        self.hex = hex
        self.flight = flight
        #self.track = track
        self.squawk = squawk
        self.lat = lat
        self.lon = lon
        #self.validposition = validposition
        self.alt_baro = alt_baro
        #self.geom_rate = geom_rate
        #self.validtrack = validtrack
        self.gs = gs 
        #self.mlat = mlat
        self.messages = messages
        self.seen = seen

    @staticmethod
    def parse_flightdata_json(json_data):
        aircraft_list = []
        ac_identified = json_data['aircraft']
        #for aircraft in json_data:
        for aircraft in ac_identified:
            if aircraft.get('flight', None) is None:
                aircraft['flight'] = "\t"
            if aircraft.get('squawk', None) is None:
                aircraft['squawk'] = ""
            if aircraft.get('lat', None) is None:
                aircraft['lat'] = 0.0
            if aircraft.get('lon', None) is None:
                aircraft['lon'] = 0.0
            if aircraft.get('alt_baro', None) is None:
                aircraft['alt_baro'] = 0.0
            if aircraft.get('gs', None) is None:
                aircraft['gs'] = 0.0

            aircraftdata = AirCraftData(
                aircraft['hex'],
                aircraft['flight'],
                #aircraft['track'],
                aircraft['squawk'],
                aircraft['lat'],
                aircraft['lon'],
                #aircraft['validposition'],
                aircraft['alt_baro'],
                #aircraft['geom_rate'],
                #aircraft['validtrack'],
                aircraft['gs'],
                #aircraft['mlat'],
                aircraft['messages'],
                aircraft['seen'])
            aircraft_list.append(aircraftdata)
        return aircraft_list
            
#test    
if __name__ == "__main__":
    #create FlightData object
    myflights = FlightData()
    while True:
        #loop through each aircraft found
        for aircraft in myflights.aircraft:
            
            #print the aircraft's data
            mydata = aircraft.hex + "\t" + aircraft.squawk + "\t" + aircraft.flight + "\t" + str(aircraft.lat) + "\t" + str(aircraft.lon) + "\t" + str(aircraft.alt_baro) + "\t" + str(aircraft.gs)+ "\t" + str(aircraft.messages) + "\t" + str(aircraft.seen)
            if aircraft.lat > 0.0 and aircraft.gs > 0.0:
                print(mydata)
        sleep(1)

        #refresh the flight data
        myflights.refresh()
