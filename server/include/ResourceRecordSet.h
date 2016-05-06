#ifndef GPUDNS_RESOURCERECORDSET_H
#define GPUDNS_RESOURCERECORDSET_H

#include <memory>
#include "ResourceRecord.h"

class ResourceRecordSet {
public:

    ResourceRecordSet(RRType rrType, std::vector<std::unique_ptr<ResourceRecord>> rrs,
                      std::unique_ptr<RRSigRecord> rrSigRecord) : rrType(rrType), rrs(std::move(rrs)),
                                                                  rrSigRecord(std::move(rrSigRecord)) {

    }

    ResourceRecordSet(ResourceRecordSet &&other) : rrType(other.rrType), rrs(std::move(other.rrs)), rrSigRecord(std::move(other.rrSigRecord)) {

    }

    RRType getRRType() {
        return rrType;
    }

    const ResourceRecord &operator[](std::size_t index) const {
        return *(rrs[index].get());
    }

    ResourceRecord &operator[](std::size_t index) {
        return *(rrs[index].get());
    }

    const ResourceRecord *get(std::size_t index) const {
        return rrs[index].get();
    }

    const size_t size() const {
        return rrs.size();
    }

    const ResourceRecord *getRRSigRecord() const {
        return rrSigRecord.get();
    }

    ~ResourceRecordSet() {

    }

private:
    RRType rrType;
    std::vector<std::unique_ptr<ResourceRecord>> rrs;
    std::unique_ptr<RRSigRecord> rrSigRecord;
};


#endif //GPUDNS_RESOURCERECORDSET_H
