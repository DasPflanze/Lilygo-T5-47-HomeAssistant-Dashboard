#ifndef HA_CLIENT_H
#define HA_CLIENT_H

#include "../common/definitions.h"

// Home Assistant API client functions
int checkOnOffState(String entity);
HAConfigurations getHaStatus();
String getSensorValue(String entity);
String getSensorAttributeValue(String entity, String attribute);
float getSensorFloatValue(String entity);

// Initialize HA client
void initHAClient();

#endif // HA_CLIENT_H