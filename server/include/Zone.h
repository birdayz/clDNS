#ifndef GPUDNS_ZONE_H
#define GPUDNS_ZONE_H

#include <vector>
#include <algorithm>
#include <string>

#include "ResourceRecord.h"
#include "LDNSWrapper.h"
#include "spdlog/spdlog.h"
#include "Label.h"
#include <tuple>

class Zone {
public:
    Zone(qname name, NSEC3Salt salt, unsigned short iterations);

    Zone(Zone &&other) : name(std::move(other.name)), salt(std::move(other.salt)),
                         nsec3Chain(std::move(other.nsec3Chain)),
                         nameSets(std::move(other.nameSets)), useDNSSEC(other.useDNSSEC), iterations(other.iterations) {
    }

    Zone(const Zone &) = delete;

    Zone &operator=(const Zone &) = delete;

    ~Zone();


    /**
     * Find record in the zone.
     * @param qname qname of the record
     * @param type  Resource Record type
     * @return pointer to const ResourceRecord or nullptr if not found
     */
    ResourceRecordSet const *findRecord(qname &name, RRType type) const;

    const NSEC3Record *findCoveringRecord(NSEC3HashB32 &domainHash) const;

    // query by NSEC3 Record domain name, e.g. 56DB<...32chars>.example.com
    const NSEC3Record *findNSEC3Record(const unsigned char *name, size_t nameSize) const;

    std::pair<const Label *, const qname> performClosestEncloserProof(qname &name) const;

    Label const *findLabel(qname &findLabel) const;

    inline const qname &getName() const {
        return name;
    }

    unsigned short getIterations() const { // TODO
        return iterations;
    }

    const NSEC3Salt &getSalt() const { // TODo
        return salt;
    }


    void addName(Label rrSet);

private:
    qname name;
    NSEC3Salt salt;
    std::vector<const NSEC3Record *> nsec3Chain;

    std::vector<Label> nameSets;

    bool useDNSSEC;
    unsigned short iterations;
};

#endif //GPUDNS_ZONE_H
