#ifndef MONITORY_MAIN_H
#define MONITORY_MAIN_H
#include <pthread.h>

class Monitor {
  pthread_cond_t full{};
  pthread_cond_t empty{};
  pthread_mutex_t mutex{};
  long int magazineState = 0;
  long int capacity = 0;

private:
  long int readFromMagazine() {
    char *pEnd;
    FILE *file = fopen("magazine.txt", "r");

    char fileContent[10];
    long int currentState = 0;

    fscanf(file, "%s", fileContent);
    fclose(file);

    currentState = strtol(fileContent, &pEnd, 10);

    return currentState;
  }

public:
  void fillMagazine(int producedItems) {
    magazineState += producedItems;
    cout << "W magazynie znajduje się " << magazineState << " sztuk towaru\n";
  }

  void takeFromMagazine(int consumedItems) {
    magazineState -= consumedItems;
    cout << "W magazynie znajduje się " << magazineState << " sztuk towaru\n";
  }

  void setCapacity(long int cap) { capacity = cap; }

  explicit Monitor() {
    int resultCode;
    resultCode = pthread_cond_init(&full, nullptr);
    assert(!resultCode);
    resultCode = pthread_cond_init(&empty, nullptr);
    assert(!resultCode);
    resultCode = pthread_mutex_init(&mutex, nullptr);
    assert(!resultCode);
  }
};
extern Monitor monitor;

#endif // MONITORY_MAIN_H
