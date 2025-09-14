#include "configurations.h"

// User configuration file definitions - declarations are in configurations.h

// WiFi credentials
const char* ssid        = "UDR";
const char* password    = "Muellerini02032017";
const char* ssid2       = "";
const char* password2   = "";

// Home Assistant server and token
const String ha_server  = "http://192.168.2.30:8123";
const String ha_token   = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI1ZTBhYzg3ZjM2Nzg0NWQyYmI0YjU5MTQ5YWQ4MmY5MSIsImlhdCI6MTc1NzcwMDc2NywiZXhwIjoyMDczMDYwNzY3fQ.-1RMo35z_Gh51A3NyEat4tU1sbWMF12Vtk_4MsRqZvA";

// GMT Offset in seconds
int gmtOffset_sec = 19800;

// Home Assistant entities
HAEntities haEntities[] {
    {"Küche", "light.kuche_light", LIGHT, ONOFF},
//    {"ROOF", "switch.tasmota_2", LIGHT, ONOFF},
//    {"FR. DOOR", "switch.tasmota_3", LIGHT, ONOFF},
//    {"BAR", "switch.exhaust_fan", EXFAN, ONOFF},
//    {"ROSE", "sensor.rose", HIGROW, VALUE},
//    {"BENJAMIN", "sensor.benjamin", HIGROW, VALUE},
//    {"M. BEDROOM", "fan.xiaomi_air_purifier_2s", AIRPURIFIER, ONOFF},
//    {"HEATER", "switch.water_heater_2", WATERHEATER, ONOFF},
//    {"FR. DOOR", "switch.tasmota_3", LIGHT, ONOFF},
//    {"BEDROOM UVC", "switch.uvc_bedroom_ac", SWITCH, ONOFF},
//    {"GARAGE", "switch.uvc_bedroom_ac", FAN, ONOFF},
//    {"M.BEDROOM", "switch.stairs_1_zigbee_switch_on_off", AIRCONDITIONER, ONOFF},
};

// Home Assistant sensors
HAEntities haSensors[] {
    {"Bad", "binary_sensor.fenster_badezimmer", WINDOW, ONOFF},
    {"Toilette", "binary_sensor.hmip_swdo_0000dd89a3483d", WINDOW, ONOFF},
    {"Balkon Essz.", "binary_sensor.hmip_swdo_0000dd89a3498f", DOOR, ONOFF},
    {"Küche", "binary_sensor.hmip_swdo_2_003660c993f2a9", WINDOW, ONOFF},
    //{"KITCHEN", "binary_sensor.kitchen_motion_sensor_ias_zone", MOTION, ONOFF},
    //{"MAIN", "binary_sensor.main_door_sensor_ias_zone", DOOR, ONOFF},
    //{"KITCHEN", "binary_sensor.kitchen_door_sensor_ias_zone", DOOR, ONOFF},
    //{"KITCHEN", "binary_sensor.kitchen_door_sensor_ias_zone", DOOR, ONOFF}
};

// Home Assistant float sensors
HAEntities haFloatSensors[] {
    //{"TOTAL ENERGY TODAY", "sensor.energy_meter_floor_03_energy_today", ENERGYMETER, VALUE},  // 1st tile
    //{"TOTAL ENERGY TODAY", "sensor.tasmota_energy_today", ENERGYMETER, VALUE},                // 1st tile
    //{"TOTAL ENERGY TODAY", "sensor.energy_meter_floor_01_energy_today", ENERGYMETER, VALUE},  // 1st tile
    {"HM-300", "sensor.hm_300_hm_300_power", ENERGYMETERPWR, VALUE},    // 2nd tile
    {"HM-300", "sensor.hm_300_ruckseite_power", ENERGYMETERPWR, VALUE},    // 2nd tile
    //{"CURRENT POWER", "sensor.energy_meter_floor_01_energy_power", ENERGYMETERPWR, VALUE},    // 2nd tile
    //{"ROOM 1 TEMP", "sensor.xiaomi_airpurifier_temp", TEMP, VALUE},                           // 3rd tile
    //{"ROOM 2 TEMP", "sensor.xiaomi_airpurifier_temp", TEMP, VALUE},                           // 4th tile
};

// Array size constants
const int haEntitiesCount = sizeof(haEntities) / sizeof(haEntities[0]);
const int haSensorsCount = sizeof(haSensors) / sizeof(haSensors[0]);
const int haFloatSensorsCount = sizeof(haFloatSensors) / sizeof(haFloatSensors[0]);