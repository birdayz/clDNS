#include "ZoneManager.h"

ZoneManager::ZoneManager(std::vector<Zone> zones) : zones(std::move(zones)) {

}

ZoneManager::~ZoneManager() {

}

const Zone *ZoneManager::getZone(const unsigned char* name, size_t size) {
    for (size_t i = 0; i < zones.size(); i++) {
        qname zoneName = zones[i].getName();
        if (memcmp(zoneName.c_str(),name,zoneName.size()) == 0) {
            return &zones[i];
        }
    }
    return nullptr;
}
