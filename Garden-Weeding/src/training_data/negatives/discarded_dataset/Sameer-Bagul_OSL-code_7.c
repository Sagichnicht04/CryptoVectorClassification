In the **Producer-Consumer problem** using counting semaphores, two semaphores are used:
- **`empty`**: Tracks the number of empty slots in the buffer.
- **`full`**: Tracks the number of filled slots in the buffer.

A mutex ensures mutual exclusion when accessing the shared buffer, preventing race conditions.

### C Program: Producer-Consumer Problem using Counting Semaphores and Mutex

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];  // Shared buffer
int in = 0;               // Points to the next empty slot
int out = 0;              // Points to the next full slot

sem_t empty;              // Counts empty slots
sem_t full;               // Counts full slots
pthread_mutex_t mutex;    // Mutex lock for critical section

void *producer(void *arg) {
    int item;
    while (1) {
        item = rand() % 100; // Produce an item
        sem_wait(&empty);    // Wait for an empty slot
        pthread_mutex_lock(&mutex); // Enter critical section
        
        // Add the item to the buffer
        buffer[in] = item;
        printf("Producer produced: %d\n", item);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // Exit critical section
        sem_post(&full);              // Increment the number of full slots

        sleep(1); // Simulate production time
    }
}

void *consumer(void *arg) {
    int item;
    while (1) {
        sem_wait(&full);   // Wait for a full slot
        pthread_mutex_lock(&mutex); // Enter critical section

        // Remove the item from the buffer
        item = buffer[out];
        printf("Consumer consumed: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // Exit critical section
        sem_post(&empty);             // Increment the number of empty slots

        sleep(2); // Simulate consumption time
    }
}

int main() {
    pthread_t prod, cons; // Producer and Consumer threads

    // Initialize the semaphores and mutex
    sem_init(&empty, 0, BUFFER_SIZE); // BUFFER_SIZE empty slots initially
    sem_init(&full, 0, 0);            // No full slots initially
    pthread_mutex_init(&mutex, NULL);

    // Create the producer and consumer threads
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    // Wait for the threads to finish (they actually won't in this infinite loop)
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // Destroy the semaphores and mutex (won't actually reach here)
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
```

### Explanation:
1. **Shared Buffer**: 
   - `buffer[]`: Circular buffer with `in` (index for producing) and `out` (index for consuming).
   
2. **Semaphores**:
   - `empty`: Counts the number of empty slots. Initialized to the buffer size.
   - `full`: Counts the number of full slots. Initialized to 0.
   
3. **Mutex**:
   - `mutex`: Ensures mutual exclusion when accessing the shared buffer to prevent race conditions.

4. **Producer**:
   - Produces an item and waits for an empty slot (`sem_wait(&empty)`).
   - Acquires the mutex to add the item to the buffer.
   - Releases the mutex and increments the full slots (`sem_post(&full)`).
   
5. **Consumer**:
   - Waits for a full slot (`sem_wait(&full)`).
   - Acquires the mutex to remove the item from the buffer.
   - Releases the mutex and increments the empty slots (`sem_post(&empty)`).
   
6. **Infinite Loop**:
   - Both the producer and consumer run in an infinite loop, simulating continuous production and consumption.

### Steps to Compile and Run the Code:

1. **Save the file** (e.g., `producer_consumer.c`).

2. **Compile the code**:
   ```bash
   gcc producer_consumer.c -o producer_consumer -lpthread
   ```

3. **Run the program**:
   ```bash
   ./producer_consumer
   ```

### Sample Output:
```
Producer produced: 47
Consumer consumed: 47
Producer produced: 59
Producer produced: 22
Consumer consumed: 59
Producer produced: 34
Consumer consumed: 22
Producer produced: 13
```

### Explanation of Output:
- The producer generates a random item and adds it to the buffer.
- The consumer removes the item from the buffer and consumes it.
- This continues in a synchronized manner, with the producer waiting if the buffer is full and the consumer waiting if the buffer is empty.

This program demonstrates the producer-consumer problem using counting semaphores and mutexes for thread synchronization.