#ifndef HA_API_H
#define HA_API_H

#include "../common/definitions.h"

// Home Assistant API functions
void toggleHomeAssistantEntity(String entityID, int entityType);
void checkAndUpdateChangedTiles();

// State tracking variables
extern String lastEntityStates[12];
extern bool entityStatesInitialized;

#endif // HA_API_H