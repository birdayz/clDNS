#ifndef GPUDNS_ZONEMANAGER_H
#define GPUDNS_ZONEMANAGER_H

#include "Zone.h"

class ZoneManager {
public:
    ZoneManager(std::vector<Zone> zones);

    ZoneManager(const ZoneManager &) = delete;

    ZoneManager &operator=(const ZoneManager &) = delete;

    ~ZoneManager();

    /**
     * @param name name of zone
     * @return zone or nullptr if not found
     */
    Zone const *getZone(const unsigned char* name, size_t size);

private:
    std::vector<Zone> zones;
};


#endif //GPUDNS_ZONEMANAGER_H
