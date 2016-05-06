#include "HashWorker.h"

void HashWorker::doWork() {
    while (!abort) {
        auto requestsTuple = std::move(hashQueue.get()->pop());
        while (requestsTuple.second == 0) {
            requestsTuple = std::move(hashQueue.get()->pop());
        }

        size_t numItems = requestsTuple.second;
        unsigned char *domains = new unsigned char[NSEC3_DOMAIN_MAX_LENGTH * BATCH_SIZE];
        unsigned char *salts = new unsigned char[NSEC3_SALT_MAX_LENGTH * BATCH_SIZE];
        dataInfoItem *data_info = new dataInfoItem[BATCH_SIZE];
        memset(data_info, 0, sizeof(dataInfoItem) * BATCH_SIZE);
        for (size_t i = 0; i < numItems; i++) {
            memcpy(&domains[i * NSEC3_DOMAIN_MAX_LENGTH], (*requestsTuple.first)[i].domainToHash.c_str(),
                   (*requestsTuple.first)[i].domainToHash.size());
            memcpy(&salts[i * NSEC3_SALT_MAX_LENGTH], (*requestsTuple.first)[i].zone->getSalt().c_str(),
                   (*requestsTuple.first)[i].zone->getSalt().size());

            data_info[i].dataLengthGlobal = NSEC3_DOMAIN_MAX_LENGTH;
            data_info[i].saltLengthGlobal = NSEC3_SALT_MAX_LENGTH;
            data_info[i].iterations = ((*requestsTuple.first)[i].zone)->getIterations();
            data_info[i].itemDomainLength = (*requestsTuple.first)[i].domainToHash.size();
            data_info[i].itemSaltLength = (*requestsTuple.first)[i].zone->getSalt().size();
        }

        DispatcherTask task;

        task.data_info = data_info;
        task.domain = &domains[0];
        task.salt = &salts[0];
        task.domainLength = NSEC3_DOMAIN_MAX_LENGTH;
        task.saltLength = NSEC3_SALT_MAX_LENGTH;
        task.batchSize = BATCH_SIZE;
        task.callback = [domains, salts, data_info, requestsTuple, this, numItems](
                std::unique_ptr <std::array<std::array < byte, 20>, BATCH_SIZE>> result) {
            for (size_t i = 0; i < numItems; i++) {
                auto hash = toBase32Hex(result->data()[i].data());

                auto finishedDNSRequest = nsec3Processor.finishProcessing(
                        std::move((*requestsTuple.first)[i].request),
                        *((*requestsTuple.first)[i].zone),
                        hash); // TODO mapping auf req safe?
                udpSender->writeResponse(std::move(finishedDNSRequest));
            }

            delete[] domains;
            delete[] salts;
            delete[] data_info;
        };

            batchDispatcher->dispatch(task);


    }
}
