#ifndef GPUDNS_TESTHELPER_H
#define GPUDNS_TESTHELPER_H

#include "Zone.h"
#include "ResourceRecord.h"
#include "Label.h"
#include <memory>
#include "base64.h"
#include "helper.h"
#include "LDNSWrapper.h"
#include <stdexcept>

// eher nameset/name nicht rrset
/*inline NameSet constructNameSet(const unsigned char *name, size_t nameLength, const unsigned char *nameNsec3,
                                size_t lengthNameNSEC3, NSEC3HashB32 nextHash, Zone &z, const std::string qnameSig,
                                const std::string nsec3Sig) {

    std::unique_ptr<RRSigRecord> rrSigNSEC3 = std::make_unique<RRSigRecord>(qname(nameNsec3, lengthNameNSEC3), 1337,
                                                                            RRclazz::IN, 2, 0,
                                                                            1420070400,

                                                                            1420070400, z.getName(),
                                                                            base64_decode(nsec3Sig));

    std::unique_ptr<const NSEC3Record> ns = std::make_unique<const NSEC3Record>(
            qname(nameNsec3, lengthNameNSEC3), 1338, RRclazz::IN,
            z.getIterations(),
            z.getSalt(), nextHash, std::move(rrSigNSEC3));

    qname rrSetName = qname(name, nameLength);

    NameSet nameSet(rrSetName, std::move(ns));

    unsigned char rdata[4] = {8, 8, 8, 8};

    std::vector<std::unique_ptr<ResourceRecord>> rrs = std::vector<std::unique_ptr<ResourceRecord>>(); // hold rrs of rrSet
    std::unique_ptr<ARecord> aRecord = std::make_unique<ARecord>(qname(name, 17), 1337, RRclazz::IN, rdata);


    std::unique_ptr<RRSigRecord> rrSig = std::make_unique<RRSigRecord>(aRecord->getName(), 1337, RRclazz::IN, 2, 0,
                                                                       1420070400,

                                                                       1420070400, z.getName(),
                                                                       base64_decode(qnameSig));

    rrs.push_back(std::move(aRecord)); // add A record

    std::unique_ptr<ResourceRecordSet> rrSet = std::make_unique<ResourceRecordSet>(RRType::A, std::move(rrs),
                                                                                   std::move(rrSig)); //init RRSET

    nameSet.add(std::move(rrSet)); //TODO den namen haben wir nun 2x, einmal im RRset und einmal im eigentlichen record.
    return std::move(nameSet);
}*/

std::unique_ptr<NSEC3Record> createNSEC3Record(const char *nsec3RRQname, NSEC3Salt nsec3Salt, const char *nsec3RRHash,
                                               const char *signature, const char *sigOwnerName, int nsec3Iterations) {

    auto nsec3Sig = std::string(
            signature);

    std::unique_ptr<RRSigRecord> rrSigNSEC3 = std::make_unique<RRSigRecord>(
            textToQname(nsec3RRQname), 1337,
            RRclazz::IN, 2, 0,
            1420070400,
            1420070400, textToQname(sigOwnerName),
            base64_decode(nsec3Sig));

    std::unique_ptr<NSEC3Record> record = std::make_unique<NSEC3Record>(textToQname(nsec3RRQname), 1337, RRclazz::IN,
                                                                        10, nsec3Salt,
                                                                        textToHash(nsec3RRHash), std::move(rrSigNSEC3));

    return std::move(record);

}


inline std::vector<std::unique_ptr<NSEC3Record>> createNSEC3Records(NSEC3Salt salt, unsigned short iterations) {
    std::vector<std::unique_ptr<NSEC3Record>> ret;

    ret.push_back(createNSEC3Record("1ED3PCJUDGJ849F6SLU38JPVVKS6S505.dnsgpu.bogus", salt,
                                    "4V7G7DABE9AD7JQC3HP8OK4G1P4K9MMV",
                                    "oFtH6H/WQdEydy1nKnXAKuzZlxwGRfAKeLbSEmG4PyKav5VUUNUuLGK9lt0lqcpg0Oh9ku6R0NPMbVDRrfdNHx05qXgLbuCwKfDwUyN/DmPQGDdE0j952CndIeBoJa9Pgc2nbzv4BO0qvUWRLA9ciSnx/lKDCmT1qlORkJNL836ZZP6hUGjsm68G1q1OyaHTKyVyn0oc7pmHyzfxW8+3OYuzmShlpPeQBwrULBd3sjr9AeoiVTsmp8+bx0UUzS0MwPjg+pvpvEZ5aYKya5w7HH4V1hAK77gH3J+1XSnQ0vO5B+nNmvGbESqouwCgsgZtU/lKqXukrEEUFs7AAZESpw==",
                                    "dnsgpu.bogus", iterations));

    ret.push_back(createNSEC3Record("JPE2BJS4O20HE96DT8LU61DV45T5QA53.dnsgpu.bogus", salt,
                                    "1ED3PCJUDGJ849F6SLU38JPVVKS6S505",
                                    "Ao1s0CW05NuUgsi4i9QetS51712jMX0wdehQo/Hrw+UF6cXzQh7l/gKSzT95XIjDfnDM3uzzNG8ZjPPrDUskPHVR97mqfQeWH7a74zS91fWk9UZlM075UeOAirbiDMaPV9ZvOx05A6Aj2P0wVNqQCgadY2epkxyR6Gcy+7kH5JDKMuj8U/z6nmOGwpy8oiVAWgUWUyGvYkph+lhx7OO898fGC9jGrbBpEc7EelFFtwlQi2S0r0zyY4vAdkFfwdH1rpBBy4khyp4H3tm6AcGk3Gny1RsH2XCsJMZO6CpnE3JE87JfGzpjN64pKHHqSNTxMTVJA+XEAjSTKNgNs2PfNg==",
                                    "dnsgpu.bogus", iterations));

    ret.push_back(createNSEC3Record("F4GOGHCJNG1I1NOSRB44I3MV0R3V73KB.dnsgpu.bogus", salt,
                                    "JPE2BJS4O20HE96DT8LU61DV45T5QA53",
                                    "IFim+vh8NdyIOJ6T/TQvgwUufQ24jRGS7CWl+ySuD0Uk2Jczi4QmQTylOwO85GQcJkMBFPVULwOqRTqVc5xc2+Y8Mvw5I3rpMdRkKfGRz5iUPfop6S4Vo6QMlNC2YkgqdBKx2ohSobxfJMkvwhwwB4fY0SOQZPaYAnGQvmZXD2C2is/1XwrKmQaJdfgR/TV1Duz5SLysPtmSvSJCRdVlxRZvvUJdSOoW3kqGBt4W/uq2mNIvhFkcrvT31u09oMkD13KC/zlepyI9mTLRVhlaJ12ztxBrlLlStn3dlUhzPecnQalD53kJgbHQpYou/a/moqusb0aVKAyqaaMJKQlmxQ==",
                                    "dnsgpu.bogus", iterations));

    ret.push_back(createNSEC3Record("AIV76ET468QEECMOPFH0NOKQ8SBPTI3R.dnsgpu.bogus", salt,
                                    "F4GOGHCJNG1I1NOSRB44I3MV0R3V73KB",
                                    "RK8nlimzfRGCuvDjhh7/h4m4Aj8Kb/n5K2iHNlqFmOrGfVizrBBpJ1t3Wa75MHayFrjWYRlcw5IvUrPGE7yyFtXNhOnPmOkk1YWQtiyUkD5mutYZb1WZ+ISHp3VRD3ZR6VBbENPcYpPiPph1iHB0AQPE3jVxfqY8L6BKugwBHNw92AhSoqsTmKD4cGiA4UDjPQClRTc3G+6rFPclJe0qz0oes/j9aPjtIIfM1gWk656qOlZKiL4UTVn0SLhTeWQjFMScbfAsJpNxiMQVl00oOi3tl++hvjYL/zimr4bAFsfsWn3ZzaLCwr6/cJK769PKTKRXTLsSEkuFamLUnCuRMg==",
                                    "dnsgpu.bogus", iterations));

    ret.push_back(createNSEC3Record("4V7G7DABE9AD7JQC3HP8OK4G1P4K9MMV.dnsgpu.bogus", salt,
                                    "AIV76ET468QEECMOPFH0NOKQ8SBPTI3R",
                                    "HWKp2ZMvvGobDftt3hVuBQVaC+FjP42yyEXLgo19Xt/wvra2zemLEfF5UaFiOAaL1Bmi+1mvEvRb0UIII/Afe2vQQXS2AbjVvewdvo/0g+C2X7gqlTC363UEOedyKUM4iO6ichefp29tZDOvgPk+Ro5sHkIQ6CICCQaofX7Y0+dfx96zL0TyG62hggsXLwYGJzcPpTI0/S6waDZeLyfPUHwtuKdlOfW509OnyoeQoj++7TZ+IhqE6xMNPv9aGuFhrtLMhkTrv/bviTGvEu4BRx08RH00y1phUxdg/bFsIzxuNd6McAMS50a4ySZlX5SFQKHqc3BJLi4f5NPLQgQtHA==",
                                    "dnsgpu.bogus", iterations));
    return ret;
}

std::unique_ptr<NSEC3Record> findNSEC3RecordForName(std::vector<std::unique_ptr<NSEC3Record>> &nsec3Records, qname name,
                                                    NSEC3Salt salt, const short unsigned iterations) {
    Nsec3HashB32Functor hash;
    NSEC3HashB32 hashedName = hash(name, salt,
                                   iterations);

    for (size_t i = 0; i < nsec3Records.size(); i++) {
        if (nsec3Records[i].get() != nullptr &&
            memcmp(&nsec3Records[i]->getName().c_str()[1], hashedName.data(), 32) == 0) {
            return std::move(nsec3Records[i]);
        }
    }
    throw std::logic_error("couldnt find nsec3 record");
}

inline Label createNameSet(const char *name, std::vector<std::unique_ptr<NSEC3Record>> &nsec3Records, NSEC3Salt salt,
                           unsigned short iterations, std::vector<const NSEC3Record *> &nsec3Chain) {
    char wildcardName[128];
    wildcardName[0] = '*';
    wildcardName[1] = '.';
    strcpy(&wildcardName[2], name);

    Nsec3HashB32Functor hash;
    NSEC3HashB32 hashedWildcardName = hash(textToQname(wildcardName), salt,
                                           iterations);

    const NSEC3Record *wildcardCoveringRecord = getCoveringRecord(nsec3Chain, hashedWildcardName);
    Label ns = Label(textToQname(name),
                     findNSEC3RecordForName(nsec3Records, textToQname(name),
                                            salt, iterations),
                     wildcardCoveringRecord);
    return ns;
}

inline Zone constructNew(unsigned short iterationsOverride = 10) {
    auto salt = textToSalt("5115CBE8100E470A");
    unsigned short iterations = 10;
    std::vector<std::unique_ptr<NSEC3Record>> nsec3Records = createNSEC3Records(salt, iterations);

    //temp only - add records to chain so we can query it
    std::vector<const NSEC3Record *> nsec3Chain;
    for (size_t i = 0; i < nsec3Records.size(); i++) {
        nsec3Chain.push_back(nsec3Records[i].get());
    }
    std::sort(nsec3Chain.begin(), nsec3Chain.end(),
              [](const NSEC3Record *a, const NSEC3Record *b) -> bool { //TODO wirklich notwendig?
                  return a->getName() < b->getName();
              });

    Label dnsgpuBogus = createNameSet("dnsgpu.bogus", nsec3Records, salt, iterations, nsec3Chain);

    Label test = createNameSet("test.dnsgpu.bogus", nsec3Records, salt, iterations, nsec3Chain);
    std::vector<std::unique_ptr<ResourceRecord>> testARRs;
    unsigned char testAIP[4] = {8, 8, 8, 8};
    testARRs.push_back(std::make_unique<ARecord>(textToQname("test.dnsgpu.bogus"), 1337, RRclazz::IN, testAIP));
    std::unique_ptr<RRSigRecord> rrSigTestA = std::make_unique<RRSigRecord>(
            textToQname("test.dnsgpu.bogus"), 1337,
            RRclazz::IN, 2, 0,
            1420070400,
            1420070400, textToQname("dnsgpu.bogus"),
            base64_decode(
                    "Tphm5Dljz8/ufv89UZ22PRVpaQmdDxy0uFhkOqxZATJu10cHLgZhteRReQ/ySWHxkXX+srTOh6/RUKeBWzJ4zPzAUY/sWG2CBUgPYLzkslUCwQoxQwFxeIbTcdkzwxU4OchcvtfpyyTz5/0jzgOVT0P6Ek3dmysu2CDnzMMf+r7lC0tfAfYYCNIGHL7QYZKbMfeX9HSxvd3zNkZ+25ZXasMU5t9Z5HktFMX0qiBzCy0gpGHMe0dO8mrqWW2cOiTjXJvdvh89UogCZe7s8jju6TZHOfhKKAD8f8afvt8BI/N5Xn6DLJXxZoWdy/6F8oLXbXBa/rB4rjnyrpZQfDVbSw=="));
    std::unique_ptr<ResourceRecordSet> testA = std::make_unique<ResourceRecordSet>(RRType::A, std::move(testARRs),
                                                                                   std::move(rrSigTestA));
    test.add(std::move(testA));

    Label test4 = createNameSet("test4.dnsgpu.bogus", nsec3Records, salt, iterations, nsec3Chain);
    std::vector<std::unique_ptr<ResourceRecord>> test4ARRs;
    unsigned char test4AIP[4] = {8, 8, 8, 8};
    test4ARRs.push_back(std::make_unique<ARecord>(textToQname("test4.dnsgpu.bogus"), 1337, RRclazz::IN, test4AIP));
    std::unique_ptr<RRSigRecord> rrSigTest4A = std::make_unique<RRSigRecord>(
            textToQname("test4.dnsgpu.bogus"), 1337,
            RRclazz::IN, 2, 0,
            1420070400,
            1420070400, textToQname("dnsgpu.bogus"),
            base64_decode(
                    "uvXgTK9DXNaPCtYg5RA+ZnAQILG7wDLtIX+/v56SFXrneDquRcK8JvAFiICES5okTsligBl30ibp7bWgDBcB7bYQRuEQroUUH0wwUUroHBiMZnh8TPjfAC1hMW86tGS0R3rE/wlHvLZf/CRZHsNdE8oXsQaPZi/VvlW+fYuOzfOULAFSsiKZJQuT+0MVLsJ8eU7Beiq0OjfBKXKl+R048G44viKUCy78SMsRHg0AmpPQAcDYSaX0NwKhPYQZ3fI5zQpX8HYgYH5EC/+SSvoqjydjywGPpUM0I3Llugv8NMPMXUzJNMrUhXt6MXgxyTyfnSkqWkK8k+i/wsSmK5sC8A=="));
    std::unique_ptr<ResourceRecordSet> test4A = std::make_unique<ResourceRecordSet>(RRType::A, std::move(test4ARRs),
                                                                                    std::move(rrSigTest4A));
    test4.add(std::move(test4A));

    Label test2 = createNameSet("test2.dnsgpu.bogus", nsec3Records, salt, iterations, nsec3Chain);
    std::vector<std::unique_ptr<ResourceRecord>> test2ARRs;
    unsigned char test2AIP[4] = {8, 8, 8, 8};
    test2ARRs.push_back(std::make_unique<ARecord>(textToQname("test2.dnsgpu.bogus"), 1337, RRclazz::IN, test2AIP));
    std::unique_ptr<RRSigRecord> rrSigTest2A = std::make_unique<RRSigRecord>(
            textToQname("test2.dnsgpu.bogus"), 1337,
            RRclazz::IN, 2, 0,
            1420070400,
            1420070400, textToQname("dnsgpu.bogus"),
            base64_decode(
                    "Gb4vIObB09TrfwKF9yPUedY5gaqlqqivRcPQHSaqexcl06rLNJNxFtu03ShmsDv/W4CTT6r0kYaGRrOBkkRlw7EEH3cSpWRTWSFK5fxF+xZLi8pGh+LbyxF1qlj4DmaWK+vjfI9VlbGi8yjrNZ/tpsUFgs6nu4y/O/dbbFZ1HTXN0Q8OSzuspIEs5V0+WAtq5ijuvS5MBKIUYcDfDrfza6/o7Fwiak7hFL3tOD/ku/feWlDLd5ziji9QanJKCy9hAEg8S0U9exQuCS6sI1JGC4junfyynJs727KrQSzEMN0dQ8TLCaQbNrgCiyGi5RFlvvAFWqim+rCN43jb+1+45g=="));
    std::unique_ptr<ResourceRecordSet> test2A = std::make_unique<ResourceRecordSet>(RRType::A, std::move(test2ARRs),
                                                                                    std::move(rrSigTest2A));
    test2.add(std::move(test2A));

    Label ns = createNameSet("ns.dnsgpu.bogus", nsec3Records, salt, iterations, nsec3Chain);
    std::vector<std::unique_ptr<ResourceRecord>> nsARRs;
    unsigned char nsAIP[4] = {8, 8, 8, 8};
    nsARRs.push_back(std::make_unique<ARecord>(textToQname("ns.dnsgpu.bogus"), 1337, RRclazz::IN, nsAIP));
    std::unique_ptr<RRSigRecord> rrSigNsA = std::make_unique<RRSigRecord>(
            textToQname("ns.dnsgpu.bogus"), 1337,
            RRclazz::IN, 2, 0,
            1420070400,
            1420070400, textToQname("dnsgpu.bogus"),
            base64_decode(
                    "HPcSEi5IBmebVbA9kq0b2dnB80y6YpV8Khu1zHWQy4foXXHJP3+i/14LOuhi9JjkDtOqh3c4tfNYQFp8X8VNLc39rWZ4+1wanyrSjgX2OX3z1+e39FYXcsbCSRF5ncLVIoSG1xtS8zMiEo/spyaQa83UT0hkeYwfp0cdDvyOKygeypTcacJ0ePq4lO9bsZncpumyIiMa/zMGDse47C43mK79CrvBZk2ICqy6tnB5su5Y11E1ooUR+ZXwduvNJHxMhVsaUmGjxRvCvr9S4zy3w/nsNhahbj8N81X46bgoLnkKJtFA/ns+bueUk0Vu4CIANVasTYKTQJ0IYvoSHfC04w=="));
    std::unique_ptr<ResourceRecordSet> nsA = std::make_unique<ResourceRecordSet>(RRType::A, std::move(nsARRs),
                                                                                 std::move(rrSigNsA));
    ns.add(std::move(nsA));

    Zone z(textToQname("dnsgpu.bogus"), salt, iterationsOverride);

    z.addName(std::move(dnsgpuBogus));
    z.addName(std::move(test));
    z.addName(std::move(test4));
    z.addName(std::move(test2));
    z.addName(std::move(ns));
    return z;
}

#endif //GPUDNS_TESTHELPER_H
