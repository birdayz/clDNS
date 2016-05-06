#ifndef GPUDNS_LABEL_H
#define GPUDNS_LABEL_H

#include "ResourceRecord.h"
#include "helper.h"
#include "ResourceRecordSet.h"
#include <unordered_map>

class Label {
public:

    Label(qname name) : rrs(), name(name), nsec3RR(nullptr) {

    }

    Label(qname name, std::unique_ptr<const NSEC3Record> nsec3RR,
            const NSEC3Record *closestEncloserWildcardCoveringRecord)
            : rrs(), name(name), nsec3RR(std::move(nsec3RR)),
              closestEncloserWildcardCoveringRecord(closestEncloserWildcardCoveringRecord) { }

    Label(const Label &other) = delete;

    Label(Label &&other) : rrs(std::move(other.rrs)), name(std::move(other.name)),
                               nsec3RR(std::move(other.nsec3RR)),
                               closestEncloserWildcardCoveringRecord(other.closestEncloserWildcardCoveringRecord) { }

    const qname &getName() const {
        return name;
    }

    ~Label() {

    }

    void add(std::unique_ptr<ResourceRecordSet> resourceRecordSet) {
        rrs[resourceRecordSet->getRRType()] = std::move(resourceRecordSet);
    }

    const ResourceRecordSet *get(RRType rrType) const {
        return rrs[rrType].get();
    }

    const NSEC3Record *getNSEC3Record() const {
        if (nsec3RR.get() == nullptr)
            std::cout << "returned nullptr NSEC3RR from RRSet" << std::endl;
        return nsec3RR.get();
    }

    const NSEC3Record *getClosestEncloserWildcardCoveringRecord() const {
        return closestEncloserWildcardCoveringRecord;
    }

private:
    mutable std::unordered_map<int, std::unique_ptr<ResourceRecordSet>> rrs;
    qname name;

    std::unique_ptr<const NSEC3Record> nsec3RR;

    // wildcard closest encloser record for *.(this->name)
    const NSEC3Record *closestEncloserWildcardCoveringRecord;
};


#endif //GPUDNS_NAMESET_H
