// mutex and posix semaphore solution
// this version runs on macOS using sem_open instead of sem_init
// sem_open uses named semaphores
#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> //required for sem_open O_CREAT
using namespace std;

struct arg{
  string input;
  string RLE;
  vector<int> freq;
  pthread_mutex_t *bsem;
  pthread_cond_t *printTurn;
  int *turn;
  int threadID;
  sem_t *semmy;
};

//struct containing output for each RLE string and frequencies
struct answer {
  string RLE;
  vector<int> freq;
};

// function running the RLE returning an answer struct
answer runEncoder(string input) {
  string result;
  vector<int> frequencies;

  int length = input.length();
  int pos1 = 0, pos2 = 1, count = 1;

  // iterate through length of string
  for (int i = 0; i < length; i++) {
    // assign first two characters of string
    char position1 = input[pos1];
    char position2 = input[pos2];

    // check if chars are matching and begin counting
    if (position1 == position2) {
      count++;
    } else {
      // add to string if count greater than 1
      if (count > 1)
      {
        frequencies.push_back(count); // add frequency count to vector
        result += position1;
        result += position1;

        // if char not repeated, add one char to string
      } else {
        result += position1;
      }
      count = 1; // reset count
    }
    // increment through the input string
    pos1++;
    pos2++;
  }

  // create answer struct and assign values
  answer ans;
  ans.RLE = result;
  ans.freq = frequencies;

  return ans;
}

//print function to output results
void print(arg x_ptr) {
    cout << "Input string: " << x_ptr.input << endl;
    cout << "RLE String: " << x_ptr.RLE << endl;
    cout << "RLE Frequencies:";
    int freqSize = x_ptr.freq.size();
    for (int j = 0; j < freqSize; j++)
      cout << " " << x_ptr.freq[j];
    cout << endl << endl;
}

//function called in pthread create
void *rleThread(void *x_void_ptr) {
    struct arg x_ptr = *((struct arg *)x_void_ptr);
    
    //signals the semaphore
    sem_post(x_ptr.semmy);

    //run RLE function and assign values
    answer ans = runEncoder(x_ptr.input);
    x_ptr.RLE = ans.RLE;
    x_ptr.freq = ans.freq;

    pthread_mutex_lock(x_ptr.bsem);
    // waits until turn matches thread ID value
    while (*x_ptr.turn != x_ptr.threadID)
      pthread_cond_wait(x_ptr.printTurn, x_ptr.bsem);
    pthread_mutex_unlock(x_ptr.bsem);

    // cout << "turn: " << *x_ptr.turn << " | threadID: " << x_ptr.threadID << endl;
    print(x_ptr);

    pthread_mutex_lock(x_ptr.bsem);
    //increment turn to move onto next thread
    (*x_ptr.turn)++;
    pthread_cond_broadcast(x_ptr.printTurn);
    pthread_mutex_unlock(x_ptr.bsem);

    return NULL;
}

int main() {
    // store input in vector
    vector<string> inputs;
    string input;
    while (getline(cin, input))
      inputs.push_back(input);

    // size is number of threads to be created
    int size = inputs.size();
    static int turn = 0;

    //initialize mutex semaphore and condition variable
    pthread_mutex_t bsem;
    pthread_mutex_init(&bsem, NULL);
    pthread_cond_t printTurn = PTHREAD_COND_INITIALIZER;
    pthread_t *tid = new pthread_t[size]; //array of threads

    //create and initialize semaphore to zero
    sem_t *semmy;
    semmy = sem_open("/Semaphore", O_CREAT, 0644, 0);

    //create and assign struct and its variables
    struct arg x;
    x.bsem = &bsem;
    x.printTurn = &printTurn;
    x.turn = &turn;
    x.semmy = semmy;

    //create threads
    for (int i = 0; i < size; i++) {
      x.input = inputs[i];
      x.threadID = i; // Assign ID for each thread
      pthread_create(&tid[i], NULL, rleThread, &x);

      //waits for semaphore to be signaled
      sem_wait(semmy);
    }

    //waits for all threads to finish then joins
    for (int i = 0; i < size; i++)
      pthread_join(tid[i], NULL);

    //close and remove semaphore
    // sem_close(semmy);
    // sem_unlink("/Semaphore");

    return 0;
}
