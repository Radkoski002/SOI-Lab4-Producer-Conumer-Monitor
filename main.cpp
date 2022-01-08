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


class Monitor
{
    pthread_cond_t full{};
    pthread_cond_t empty{};
    long int magazineState = 0;
    long int capacity = 0;
    queue<int> producerQueue;
    queue<int> consumerQueue;

private:

    long int readFromMagazine() {
        char * pEnd;
        FILE* file = fopen ("magazine.txt", "r");

        char fileContent[10];
        long int currentState = 0;

        fscanf(file, "%s", fileContent);
        fclose (file);

        currentState = strtol(fileContent, &pEnd, 10);

        return currentState;
    }

public:

    void fillMagazine(int producedItems, producer prod){
        if (capacity < magazineState + producedItems)
        {
            //printf("\tProducent zatrzymał się\n");
            producerQueue.push(producedItems);
            pthread_cond_wait(&full, &prod.mutex);
        }
        magazineState += producedItems;
        printf("\tW magazynie znajduje się %d sztuk towaru\n", magazineState);
        if(!consumerQueue.empty())
        {
            if(magazineState - consumerQueue.front() >= 0 && magazineState > capacity / 2)
            {
                //printf("\tKonsument wznowił pracę\n");
                consumerQueue.pop();
                pthread_cond_signal(&empty);
            }
        }
    }

    void takeFromMagazine(int consumedItems, consumer cons){
        if (0 > magazineState - consumedItems)
        {
            //printf("\tKosnument zatrzymał się\n");
            consumerQueue.push(consumedItems);
            pthread_cond_wait(&empty, &cons.mutex);
        }
        magazineState -= consumedItems;
        printf("\tW magazynie znajduje się %d sztuk towaru\n", magazineState);
        if(!producerQueue.empty())
        {
            if(magazineState + producerQueue.front() <= capacity && magazineState <= capacity / 2)
            {
                //printf("\tProducent wznowił pracę\n");
                producerQueue.pop();
                pthread_cond_signal(&full);
            }
        }
    }

    void setCapacity(long int cap)
    {
        capacity = cap;
    }

    long int getCapacity() const
    {
        return capacity;
    }

    long int getMagazineState() const
    {
        return magazineState;
    }

    explicit Monitor(){
        int resultCode;
        resultCode = pthread_cond_init(&full, nullptr);
        assert(!resultCode);
        resultCode = pthread_cond_init(&empty, nullptr);
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
        //printf("Producent %d próbuje wyprodukować %d towarów\n", p.id, producedItems);
        monitor.fillMagazine(producedItems, p);
        //printf("Producent %d wyprodukował %d towarów\n", p.id, producedItems);
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
        //printf("Konusment %d próbuje zabrać %d towarów\n", c.id, consumedItems);
        monitor.takeFromMagazine(consumedItems, c);
        //printf("Konusment %d zabrał %d towarów\n", c.id, consumedItems);
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
