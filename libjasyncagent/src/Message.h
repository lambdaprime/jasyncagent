#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <pthread.h>
#include <map>

typedef void (*dtor_t)(void*);

class ThreadStorageManager {

    std::map<pthread_key_t, std::pair<void*, dtor_t>> data;

public:

    bool hasKey(const pthread_key_t& k) {
        return data.find(k) != data.end();
    }

    void* getData(const pthread_key_t& k) {
        return data[k].first;
    }

    void setData(const pthread_key_t& k, void* v) {
        data[k].first = v;
    }

    void addKey(const pthread_key_t& k, dtor_t dtor) {
        data[k].second = dtor;
    }
};

struct Message {
    pthread_t id;
    bool isCoroutineActive;
    void* stackAddr;
    int stackSize;
    bool isPreloadSuccessful;
    ThreadStorageManager* tsm;
};

static const char* MESSAGE_ID_VAR = "jasyncagentid";

#endif /* MESSAGE_H_ */
