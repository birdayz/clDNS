#include "NSEC3Processor.h"

void NSEC3Processor::startProcessing(std::unique_ptr<DNS_REQUEST> request, const Zone &zone) {
    auto labelTuple = zone.performClosestEncloserProof(request->domain);

    auto labelClosestEncloser = labelTuple.first;
    auto labelNextCloser = labelTuple.second;

    // Add closest encloser NSEC3 record
    auto NSEC3RecordClosestEncloser = labelClosestEncloser->getNSEC3Record();
    request->nsec3Records[0] = NSEC3RecordClosestEncloser;

    // Add wildcard NSEC3 record
    auto NSEC3RecordWildcard = labelClosestEncloser->getClosestEncloserWildcardCoveringRecord();
    request->nsec3Records[1] = NSEC3RecordWildcard;

    // Hash request for next closer
    HashRequest req(std::move(request), std::move(labelTuple.second), &zone);
    hashQueue->push(std::move(req));
}

std::unique_ptr<DNS_REQUEST> NSEC3Processor::finishProcessing(std::unique_ptr<DNS_REQUEST> request, const Zone &zone,
                                              NSEC3HashB32 hash) {
    auto NSEC3RecordNextCloser = zone.findCoveringRecord(hash);
    request->nsec3Records[2] = NSEC3RecordNextCloser;

   return request;
}

std::unique_ptr<DNS_REQUEST> NSEC3Processor::processSync(std::unique_ptr<DNS_REQUEST> request, const Zone &zone) {
    auto labelTuple = zone.performClosestEncloserProof(request->domain);

    auto labelClosestEncloser = labelTuple.first;
    auto labelNextCloser = labelTuple.second;

    // Add closest encloser NSEC3 record
    auto NSEC3RecordClosestEncloser = labelClosestEncloser->getNSEC3Record();
    request->nsec3Records[0] = NSEC3RecordClosestEncloser;

    // Add wildcard NSEC3 record
    auto NSEC3RecordWildcard = labelClosestEncloser->getClosestEncloserWildcardCoveringRecord();
    request->nsec3Records[1] = NSEC3RecordWildcard;

    // Hash via LDNS
    Nsec3HashB32Functor hashFunctor = Nsec3HashB32Functor();
    auto hash = hashFunctor(request->domain, zone.getSalt(), zone.getIterations());

    auto NSEC3RecordNextCloser = zone.findCoveringRecord(hash);
    request->nsec3Records[2] = NSEC3RecordNextCloser;
    return request;
}