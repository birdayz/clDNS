#include <include/LDNSWrapper.h>
#include "Zone.h"

Zone::Zone(qname name, NSEC3Salt salt, unsigned short iterations) : name(name), salt(salt), nsec3Chain(), nameSets(),
                                                                    useDNSSEC(true),
                                                                    iterations(iterations) {

}

Zone::~Zone() {

}

std::ostream &operator<<(std::ostream &os, const qname &t) {
    std::string s;
    for (unsigned int i = 0; i < t.size(); i++) {
        // std::cout << t[i];
        if (t[i] < 33) {
            os << '.';
        }
        else {
            os << (const char) t[i];
            //   os << reinterpret_cast<const char*>(t[i]);
        }
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const NSEC3HashB32 &t) {
    for (unsigned int i = 0; i < 32; i++) {
        os << (const char) t[i];
    }
    return os;
}

const NSEC3Record *Zone::findCoveringRecord(NSEC3HashB32 &domainHash) const {
    return getCoveringRecord(nsec3Chain, domainHash);
}

// return closest encloser and next closer
std::pair<const Label *, const qname> Zone::performClosestEncloserProof(qname &name) const {
    const unsigned char *seek = name.c_str();
    int bytesSkipped = *seek + 1;
    seek += *seek + 1;
    const unsigned char *previousEncloserPtr = name.c_str() + 0;
    size_t bytesSkippedPrev = 0;
    while (*seek != 0) {
        int currentLabelLength = *seek;
        const unsigned char *encloserPtr = seek;

        size_t encloserPtrLen = name.size() - bytesSkipped;
        size_t previousEncloserPtrLen = name.size() - bytesSkippedPrev;

        qname closestEncloserName(encloserPtr, encloserPtrLen);
        const qname nextCloserName(previousEncloserPtr, previousEncloserPtrLen);

        const Label *labelClosestEncloser = findLabel(closestEncloserName);
        if (labelClosestEncloser != nullptr) { // Check if label exists
            return std::make_pair(labelClosestEncloser, std::move(nextCloserName));
        }

        // Proceed through the input name by cutting off a label
        bytesSkipped += currentLabelLength + 1;
        bytesSkippedPrev += *previousEncloserPtr + 1;
        previousEncloserPtr += *previousEncloserPtr + 1;
        seek += currentLabelLength + 1;

    }
    // throw error because there MUST be a closest encloser
    throw std::logic_error("no closest encloser found");
}

const ResourceRecordSet *Zone::findRecord(qname &name, RRType type) const {
    for (const Label &nameSet : nameSets) {
        if (nameSet.getName() == name) {
            //return static_cast<const ARecord *>(rrSet.get(RRType::A));
            const ResourceRecordSet *rrSet = nameSet.get(type);
            return rrSet;
        }
    }
    // RR not found
    return nullptr;
}

const NSEC3Record *Zone::findNSEC3Record(const unsigned char *name, size_t nameSize) const {
    for (unsigned int i = 0; i < nsec3Chain.size(); i++) {
        //if (nsec3Chain[i].getName().c_str() == name) {
        if (nameSize == nsec3Chain[i]->getName().size() &&
            memcmp(name, nsec3Chain[i]->getName().c_str(), nameSize) == 0)
            return nsec3Chain[i];
    }
    return nullptr;
}

Label const *Zone::findLabel(qname &findLabel) const {
    for (const Label &nameSet : nameSets) {
        if (nameSet.getName() == findLabel) {
            return &nameSet;
        }
    }
    return nullptr;
}


void Zone::addName(Label rrSet) {
    nsec3Chain.push_back(rrSet.getNSEC3Record());
    nameSets.push_back(std::move(rrSet));
    std::sort(nsec3Chain.begin(), nsec3Chain.end(),
              [](const NSEC3Record *a, const NSEC3Record *b) -> bool {
                  return a->getName() < b->getName();
              });
}