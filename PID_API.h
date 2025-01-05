#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "nazev";         // nazev
const char* password = "heslo";     //heslo
const char* accessToken = "golemio key";  //golemio key

#define MAX_VEHICLES 20 // max pocet nactenych autobusu

bool vypnout = 0;

struct VehicleInfo {
  String gtfs_trip_id;          // gtfs
  String vehicle_id;            // ID
  String trip_headsign;         // smer
  String first_stop;            // prvni zastavka
  String last_stop;             // posledni zastavka
  String next_stop;             // dalsi zastavka
  String arrival_time;          // cas prijezdu na konecnou
  String kly_arrival_time;      // cas prijezdu na Kly,Vinice
  String vehicle_type;          // typ vozu
  int vehicle_delay;            // zpozdeni v s
  int last_stop_sequence;       // cislo posledni zastavky
  int zone_id;                  // pasmo
};

int nejBus = 0;       // nejblizsi autobus smer Ladvi

VehicleInfo vehicles[MAX_VEHICLES]; // pole jednotlivych autobusu

int najdiBus();
int getZone(VehicleInfo& vehicle);
int getAllVehiclesID(VehicleInfo vehicles[], int maxVehicles);
void getVehicleInfo(VehicleInfo& vehicle);
void getVehicleType(VehicleInfo& vehicle);
String removeDiacritics(String input, int pocZnaku);

void wifiSetup() {                        // pripojeni k wifi
  WiFi.begin(ssid, password, 6);
  Serial.print("Connecting to Wi-Fi");
  int wifiCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifiCounter++;
    if(wifiCounter >= 20) {
      vypnout = 1;
      break;
    }
  }
  Serial.println("\nConnected to Wi-Fi!");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int najdiBus() {

  int adepti[10];
  int vzdalenost[10];
  int pocetAdeptu = 0;
  int nejblizsi = 0;

  int vehicleCount = getAllVehiclesID(vehicles, MAX_VEHICLES);

  Serial.print("Total vehicles: ");
  Serial.println(vehicleCount);

  for (int i = 0; i < vehicleCount; i++) {

    getVehicleInfo(vehicles[i]);                // scope info

    //Serial.print("Vehicle "); Serial.print(i+1); Serial.print(": "); Serial.println(vehicles[i].vehicle_id);
    //Serial.print("Delay: "); Serial.println(vehicles[i].vehicle_delay);

    if (vehicles[i].trip_headsign.startsWith("P")) {
      //Serial.println("Trip headsign starts with P!");

      //Serial.print("last_stop_sequence: ");
      //Serial.println(vehicles[i].last_stop_sequence);
      if((vehicles[i].last_stop_sequence != 0) || (vehicles[i].last_stop_sequence != -1)) {
        int zone = getZone(vehicles[i]);
        //Serial.print("Zona: "); Serial.println(zone);
        if((zone == 4) || (zone == 5) || (zone == 6)) {
          Serial.println("Zone > 3!");
          adepti[pocetAdeptu] = i;
          pocetAdeptu++;
        }
      }
    }
  }

  if(pocetAdeptu != 0) {
    if(pocetAdeptu == 1) {
      Serial.print("Mam ho!  i = "); Serial.println(adepti[0]);
      //Serial.println(vehicles[adepti[0]].trip_headsign);
      Serial.println(vehicles[adepti[0]].vehicle_id);
      //Serial.println(vehicles[adepti[0]].gtfs_trip_id);
      //Serial.println(vehicles[adepti[0]].last_stop_sequence);
      //Serial.println(vehicles[adepti[0]].vehicle_delay);
      return adepti[0];
    } else {
      
      for(int j = 0; j < pocetAdeptu; j++) {
        if (vehicles[adepti[j]].first_stop.length() > 4 && vehicles[adepti[j]].first_stop[3] == 't') {
          vzdalenost[j] = 24 - vehicles[adepti[j]].last_stop_sequence;
        }
        if (vehicles[adepti[j]].first_stop.length() > 4 && vehicles[adepti[j]].first_stop[3] == 'n') {
          vzdalenost[j] = 12 - vehicles[adepti[j]].last_stop_sequence;
        }
      }
      for(int j = 1; j < pocetAdeptu; j++) {
        if(vzdalenost[j] < vzdalenost[nejblizsi]) nejblizsi = j;
      }
      Serial.print("Mam ho!  i = "); Serial.println(nejblizsi);
      //Serial.println(vehicles[nejblizsi].trip_headsign);
      Serial.println(vehicles[nejblizsi].vehicle_id);
      //Serial.println(vehicles[nejblizsi].gtfs_trip_id);
      //Serial.println(vehicles[nejblizsi].last_stop_sequence);
      //Serial.println(vehicles[nejblizsi].vehicle_delay);
      return adepti[nejblizsi];
    }
  } else {return 666;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getZone(VehicleInfo& vehicle) {
  int zone = 0;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi is not connected!");
    return 100;
  }

  HTTPClient http;

  String url = "https://api.golemio.cz/v2/public/vehiclepositions/" 
               + vehicle.vehicle_id 
               + ";gtfsTripId=" 
               + vehicle.gtfs_trip_id 
               + "?scopes=stop_times";
  http.begin(url);

  http.addHeader("X-Access-Token", accessToken);

  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    Serial.print("HTTP request failed, code: ");
    Serial.println(httpResponseCode);
    http.end();
    return 100;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<8192> doc;     // velikost
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return 101;
  }

  JsonArray features = doc["stop_times"]["features"].as<JsonArray>();

  for (JsonObject feature : features) {

    if(feature["properties"]["stop_sequence"].as<int>() == 1) {
      vehicle.first_stop = feature["properties"]["stop_name"].as<String>();
      //Serial.println(vehicle.first_stop);
    }

    if(feature["properties"]["stop_sequence"].as<int>() == vehicle.last_stop_sequence) {
      vehicle.zone_id = feature["properties"]["zone_id"].as<int>();
      vehicle.last_stop = feature["properties"]["stop_name"].as<String>();
      zone = feature["properties"]["zone_id"].as<int>();
    }
    if(feature["properties"]["stop_sequence"].as<int>() == vehicle.last_stop_sequence+1) {
      vehicle.next_stop = feature["properties"]["stop_name"].as<String>();
    }
    if(feature["properties"]["stop_name"].as<String>() == "Kly,Vinice") {
      vehicle.kly_arrival_time = feature["properties"]["arrival_time"].as<String>();
    }
    if(feature["properties"]["stop_name"].as<String>() == "Ládví") {
      vehicle.arrival_time = feature["properties"]["arrival_time"].as<String>();
      return zone;
    }
  }

  return 102;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getAllVehiclesID(VehicleInfo vehicles[], int maxVehicles) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi is not connected!");
    return 0;
  }

  HTTPClient http;
  String url = "https://api.golemio.cz/v2/public/vehiclepositions?routeShortName=369&routeType=bus";
  http.begin(url);

  http.addHeader("X-Access-Token", accessToken);

  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    Serial.print("HTTP request failed, code: ");
    Serial.println(httpResponseCode);
    http.end();
    return 0;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<4096> doc;   // velikost
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return 0;
  }

  JsonArray features = doc["features"].as<JsonArray>();
  int vehicleCount = 0;

  for (JsonObject feature : features) {
    if (vehicleCount >= maxVehicles) {
      Serial.println("Maximum vehicle limit reached.");
      break;
    }

    vehicles[vehicleCount].gtfs_trip_id = feature["properties"]["gtfs_trip_id"].as<String>();
    vehicles[vehicleCount].vehicle_id = feature["properties"]["vehicle_id"].as<String>();
    vehicleCount++;
  }

  return vehicleCount; // pocet nalezenych autobusu
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getVehicleInfo(VehicleInfo& vehicle) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi is not connected!");
    return;
  }

  HTTPClient http;

  // sestaveni URL
  String url = "https://api.golemio.cz/v2/public/vehiclepositions/" 
               + vehicle.vehicle_id 
               + ";gtfsTripId=" 
               + vehicle.gtfs_trip_id 
               + "?scopes=info";
  http.begin(url);

  http.addHeader("X-Access-Token", accessToken);

  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    Serial.print("HTTP request failed, code: ");
    Serial.println(httpResponseCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<1024> doc;   // velikost
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // nacteni dat z odpovedi
  vehicle.trip_headsign = doc["trip_headsign"].as<String>();
  vehicle.vehicle_delay = doc["delay"].isNull() ? 0 : doc["delay"].as<int>();
  vehicle.last_stop_sequence = doc["last_stop_sequence"].isNull() ? -1 : doc["last_stop_sequence"].as<int>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getVehicleType(VehicleInfo& vehicle) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi is not connected!");
    return;
  }

  HTTPClient http;

  // sestaveni URL
  String url = "https://api.golemio.cz/v2/public/vehiclepositions/" 
               + vehicle.vehicle_id 
               + ";gtfsTripId=" 
               + vehicle.gtfs_trip_id 
               + "?scopes=vehicle_descriptor";
  http.begin(url);

  http.addHeader("X-Access-Token", accessToken);

  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    Serial.print("HTTP request failed, code: ");
    Serial.println(httpResponseCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<1024> doc;   // velikost
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // nacteni dat z odpovedi
  vehicle.vehicle_type = doc["vehicle_descriptor"]["vehicle_type"].as<String>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String removeDiacritics(String input, int pocZnaku) {
  String output = "";

  int pocetZnaku = 0;
  
  int i = 0;
  while (i < input.length()) {
    unsigned char c = input[i];
    int charLen = 1;
    if ((c & 0xE0) == 0xC0) charLen = 2; // 2-bajtovy znak
    else if ((c & 0xF0) == 0xE0) charLen = 3; // 3-bajtovy znak
    else if ((c & 0xF8) == 0xF0) charLen = 4; // 4-bajtovy znak

    // cely znak
    String utf8Char = input.substring(i, i + charLen);

    // nahrazeni
    if (utf8Char == "á" || utf8Char == "ä") output += "a";
    else if (utf8Char == "č") output += "c";
    else if (utf8Char == "ď") output += "d";
    else if (utf8Char == "é" || utf8Char == "ě") output += "e";
    else if (utf8Char == "í") output += "i";
    else if (utf8Char == "ň") output += "n";
    else if (utf8Char == "ó" || utf8Char == "ö") output += "o";
    else if (utf8Char == "ř") output += "r";
    else if (utf8Char == "š") output += "s";
    else if (utf8Char == "ť") output += "t";
    else if (utf8Char == "ú" || utf8Char == "ů" || utf8Char == "ü") output += "u";
    else if (utf8Char == "ý") output += "y";
    else if (utf8Char == "ž") output += "z";
    else if (utf8Char == "Á" || utf8Char == "Ä") output += "A";
    else if (utf8Char == "Č") output += "C";
    else if (utf8Char == "Ď") output += "D";
    else if (utf8Char == "É" || utf8Char == "Ě") output += "E";
    else if (utf8Char == "Í") output += "I";
    else if (utf8Char == "Ň") output += "N";
    else if (utf8Char == "Ó" || utf8Char == "Ö") output += "O";
    else if (utf8Char == "Ř") output += "R";
    else if (utf8Char == "Š") output += "S";
    else if (utf8Char == "Ť") output += "T";
    else if (utf8Char == "Ú" || utf8Char == "Ů" || utf8Char == "Ü") output += "U";
    else if (utf8Char == "Ý") output += "Y";
    else if (utf8Char == "Ž") output += "Z";
    else if (utf8Char == "'" || utf8Char == "’" || utf8Char == "‘" || utf8Char == "′") output += (char)39;
    else {
      // bezezmeny
      output += utf8Char;
    }

    // posun indexu o delku znaku
    i += charLen;
    pocetZnaku++;
    if(pocetZnaku == pocZnaku) break;
  }
  return output;
}
