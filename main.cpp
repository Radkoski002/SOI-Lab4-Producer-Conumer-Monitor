#include <iostream>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <queue>
#include <cassert>
#include <unistd.h>

using namespace std;

struct producer{
    long int a;
    long int b;
    int id;
    pthread_mutex_t mutex;
};

struct consumer{
    long int c;
    long int d;
    int id;
    pthread_mutex_t mutex;
};

static void clearLogs()
{
    FILE* file = fopen ("prodLog.txt", "w");
    fprintf(file, "");
    fclose (file);
    file = fopen ("consLog.txt", "w");
    fprintf(file, "");
    fclose (file);
    file = fopen ("magazineLog.txt", "w");
    fprintf(file, "");
    fclose (file);
}

static void logProdTry(producer p, long int items)
{
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d próbuje wstawić %ld towarów\n", p.id, items);
    fclose (file);
}

static void logProdSuccess(producer p, long int items)
{
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d wstawił %ld towarów\n", p.id, items);
    fclose (file);
}

static void logProdStop(producer p){
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d zatrzymał się\n", p.id);
    fclose (file);
}

static void logProdResume(int id){
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d wznowił pracę\n", id);
    fclose (file);
}

static void logProdHeurStop(producer p){
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d zatrzymał się — stan magazynu\n", p.id);
    fclose (file);
}

static void logProdHeurResume(int id){
    FILE* file = fopen ("prodLog.txt", "a");
    fprintf(file, "Producent %d wznowił pracę — stan magazynu\n", id);
    fclose (file);
}

static void logConsTry(consumer c, long int items)
{
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d próbuje zabrać %ld towarów\n", c.id, items);
    fclose (file);
}

static void logConsSuccess(consumer c, long int items)
{
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d zabrał %ld towarów\n", c.id, items);
    fclose (file);
}

static void logConsStop(consumer c){
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d zatrzymał się\n", c.id);
    fclose (file);
}

static void logConsResume(int id){
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d wznowił pracę\n", id);
    fclose (file);
}

static void logConsHeurStop(consumer c){
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d zatrzymał się — stan magazynu\n", c.id);
    fclose (file);
}

static void logConsHeurResume(int id){
    FILE* file = fopen ("consLog.txt", "a");
    fprintf(file, "Konsument %d wznowił pracę — stan magazynu\n", id);
    fclose (file);
}

static void logMagazineChange(long int items){
    FILE* file = fopen ("magazineLog.txt", "a");
    fprintf(file, "Stan magazynu: %d\n", items);
    fclose (file);
}


struct queueNode
{
    int items;
    int id;
};

class Monitor
{
    pthread_cond_t full{};
    pthread_cond_t empty{};
    pthread_cond_t lessThanHalf{};
    pthread_cond_t moreThanHalf{};
    long int magazineState = 0;
    long int capacity = 0;
    queue<queueNode> producerQueue;
    queue<queueNode> consumerQueue;
    queue<queueNode> prodHeuristic;
    queue<queueNode> consHeuristic;

private:

    static void updateMagazine(long int magazineState) {
        FILE* file = fopen ("magazine.txt", "w");
        fprintf(file, "%ld", magazineState);
        fclose (file);
    }


public:

    void fillMagazine(int producedItems, producer prod){
        while (capacity < magazineState + producedItems)
        {
            if(!consHeuristic.empty()) {
                logConsHeurResume(consHeuristic.front().id);
                consHeuristic.pop();
                pthread_cond_signal(&lessThanHalf);
            }
            logProdStop(prod);
            queueNode proc = {producedItems, prod.id};
            producerQueue.push(proc);
            pthread_cond_wait(&full, &prod.mutex);
        }
        while (magazineState > capacity / 2 && consumerQueue.empty())
        {
            if(!consHeuristic.empty()) {
                logConsHeurResume(consHeuristic.front().id);
                consHeuristic.pop();
                pthread_cond_signal(&lessThanHalf);
            }
            logProdHeurStop(prod);
            queueNode proc = {producedItems, prod.id};
            prodHeuristic.push(proc);
            pthread_cond_wait(&moreThanHalf, &prod.mutex);
        }
        magazineState += producedItems;
        updateMagazine(magazineState);
        logMagazineChange(magazineState);
        if(!consumerQueue.empty())
        {
            if(magazineState - consumerQueue.front().items >= 0)
            {
                logConsResume(consumerQueue.front().id);
                consumerQueue.pop();
                pthread_cond_signal(&empty);
            }
        }
    }

    void takeFromMagazine(int consumedItems, consumer cons){
        while (0 > magazineState - consumedItems)
        {
            if(!prodHeuristic.empty()) {
                logProdHeurResume(prodHeuristic.front().id);
                prodHeuristic.pop();
                pthread_cond_signal(&moreThanHalf);
            }
            logConsStop(cons);
            queueNode proc = {consumedItems, cons.id};
            consumerQueue.push(proc);
            pthread_cond_wait(&empty, &cons.mutex);
        }
        while (magazineState <= capacity / 2 && producerQueue.empty())
        {
            if(!prodHeuristic.empty()) {
                logProdHeurResume(prodHeuristic.front().id);
                prodHeuristic.pop();
                pthread_cond_signal(&moreThanHalf);
            }
            logConsHeurStop(cons);
            queueNode proc = {consumedItems, cons.id};
            consHeuristic.push(proc);
            pthread_cond_wait(&lessThanHalf, &cons.mutex);
        }
        magazineState -= consumedItems;
        updateMagazine(magazineState);
        logMagazineChange(magazineState);
        if(!producerQueue.empty())
        {
            if(magazineState + producerQueue.front().items <= capacity)
            {
                logProdResume(producerQueue.front().id);
                producerQueue.pop();
                pthread_cond_signal(&full);
            }
        }
    }

    void setCapacity(long int cap)
    {
        capacity = cap;
    }

    explicit Monitor(){
        int resultCode;
        resultCode = pthread_cond_init(&full, nullptr);
        assert(!resultCode);
        resultCode = pthread_cond_init(&empty, nullptr);
        assert(!resultCode);
        resultCode = pthread_cond_init(&moreThanHalf, nullptr);
        assert(!resultCode);
        resultCode = pthread_cond_init(&lessThanHalf, nullptr);
        assert(!resultCode);
    }
};

Monitor monitor;

[[noreturn]] void *produce(void *arguments)
{
    srand((unsigned int) time(nullptr));
    struct producer p = *((struct producer *)arguments);
    int producedItems;
    while(true)
    {

        sleep(1);
        producedItems = rand() % (p.b - p.a + 1) + p.a;
        logProdTry(p, producedItems);
        monitor.fillMagazine(producedItems, p);
        logProdSuccess(p, producedItems);
    }
}

[[noreturn]] void *consume(void *arguments)
{

    srand((unsigned int) time(nullptr) + 1);
    struct consumer c = *((struct consumer *)arguments);
    int consumedItems;
    while(true)
    {
        sleep(1);
        consumedItems = rand() % (c.d - c.c + 1) + c.c;
        logConsTry(c, consumedItems);
        monitor.takeFromMagazine(consumedItems, c);
        logConsSuccess(c, consumedItems);
    }
}

int main(int argc, char* argv[]) {

    char * pEnd;

    long int k = strtol(argv[1], &pEnd, 10);
    long int m = strtol(argv[2], &pEnd, 10);
    long int n = strtol(argv[3], &pEnd, 10);
    long int a = strtol(argv[4], &pEnd, 10);
    long int b = strtol(argv[5], &pEnd, 10);
    long int c = strtol(argv[6], &pEnd, 10);
    long int d = strtol(argv[7], &pEnd, 10);

    monitor.setCapacity(k);

    int resultCode;
    pthread_t threads[m + n];
    struct producer producerThreads[m];
    struct consumer consumerThreads[n];
    int i = 0;

    clearLogs();

    for (i ; i < m ; i++) {
        struct producer prod = {a, b, i};
        pthread_mutex_init(&prod.mutex, nullptr);
        producerThreads[i] = prod;
        resultCode = pthread_create(&threads[i], nullptr, produce, &producerThreads[i]);
        assert(!resultCode);
    }

    for (i = 0 ; i < n ; i++) {
        struct consumer cons = {c, d, i};
        pthread_mutex_init(&cons.mutex, nullptr);
        consumerThreads[i] = cons;
        resultCode = pthread_create(&threads[i + m], nullptr, consume, &consumerThreads[i]);
        assert(!resultCode);
    }

    for (i = 0 ; i < m + n ; i++)
    {
        resultCode = pthread_join(threads[i], nullptr);
        assert(!resultCode);
    }
}
