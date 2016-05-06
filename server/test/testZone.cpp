#include <include/NSEC3Processor.h>
#include "gtest/gtest.h"
#include "ResourceRecord.h"
#include "Zone.h"
#include "testHelper.h"


TEST(TEST_ZONE, test_closest_encloser) {
    // Arrange
    Zone z = constructNew(10);

    qname testDomain(textToQname("bla.xy.dnsgpu.bogus"));

    // Act
    auto tuple = z.performClosestEncloserProof(testDomain);
    auto closestEncloser = tuple.first;
    auto nextCloser = tuple.second;

    // Assert
    // Check closest encloser name
    EXPECT_EQ(closestEncloser->getName(), textToQname("dnsgpu.bogus"));
    // Check next closer
    EXPECT_EQ(nextCloser, textToQname("xy.dnsgpu.bogus"));
}

TEST(TEST_NSEC3_PROCESSOR, test_start_finish) {
    // Arrange
    Zone z = constructNew(10);
    Nsec3HashB32Functor hashFunctor = Nsec3HashB32Functor();

    auto hashQueue = std::make_shared<HashQueue<HashRequest, BATCH_SIZE>>();
    NSEC3Processor p(hashQueue);

    // Input data
    unsigned int iterations = 10;
    auto salt = textToSalt("5115CBE8100E470A");
    qname testDomain(textToQname("test3.dnsgpu.bogus"));
    auto domainHash = hashFunctor(testDomain.c_str(), testDomain.size(), salt, iterations);

    auto expectedNSEC3Record = createNSEC3Record("JPE2BJS4O20HE96DT8LU61DV45T5QA53.dnsgpu.bogus", salt,
                                                 "1ED3PCJUDGJ849F6SLU38JPVVKS6S505",
                                                 "not_relevant_here",
                                                 "dnsgpu.bogus", iterations);
    auto fakeDNSRequest = std::make_unique<DNS_REQUEST>();
    fakeDNSRequest->domain = testDomain;

    // Act
    auto result = p.finishProcessing(std::move(fakeDNSRequest), z, domainHash);

    // Assert
    EXPECT_EQ(result->nsec3Records[2]->getName(), expectedNSEC3Record->getName());
    EXPECT_EQ(result->nsec3Records[2]->getHash(), expectedNSEC3Record->getHash());
}

// Covers wrap-around on the right side corner case
TEST(TEST_ZONE, test_nsec3chain_lookup) {
    // Arrange
    Zone z = constructNew(10);
    auto domain = textToQname("test3.dnsgpu.bogus");
    auto salt = textToSalt("5115CBE8100E470A");
    unsigned int iterations = 10;

    Nsec3HashB32Functor hashFunctor = Nsec3HashB32Functor();
    auto domainHash = hashFunctor(domain.c_str(), domain.size(), salt, iterations);

    // Act
    auto coveringRecord = z.findCoveringRecord(domainHash);

    auto expectedNSEC3Record = createNSEC3Record("JPE2BJS4O20HE96DT8LU61DV45T5QA53.dnsgpu.bogus", salt,
                                                 "1ED3PCJUDGJ849F6SLU38JPVVKS6S505",
                                                 "not_relevant_here",
                                                 "dnsgpu.bogus", iterations);

    // Assert
    EXPECT_EQ(expectedNSEC3Record->getName(), coveringRecord->getName());
    EXPECT_EQ(expectedNSEC3Record->getHash(), coveringRecord->getHash());
}