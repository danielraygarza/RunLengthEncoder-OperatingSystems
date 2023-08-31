#include<iostream>
#include<vector>
#include<string>
#include <pthread.h>
using namespace std;

//struct containing output for each RLE string and frequencies
struct answer {
    string RLE;
    vector<int> freq;
};

struct arg {
    string s;
    string RLE;
    vector<int> freq;
};

//function running the RLE returning an answer struct
answer runEncoder (string input){
    string result;
    vector<int> frequencies;

    int length = input.length();
    int pos1 = 0, pos2 = 1, count = 1;

    //iterate through length of string
    for(int i=0; i < length; i++){
        //assign first two characters of string
        char position1 = input[pos1];
        char position2 = input[pos2];

        //check if chars are matching and begin counting
        if(position1 == position2){
            count++;
        } else {
            //add to string if count greater than 1 
            if(count > 1){
                frequencies.push_back(count); //add frequency count to vector
                result += position1;
                result += position1;

            //if char not repeated, add one char to string
            } else {
                result += position1;
            }
            count = 1; //reset count
        }
        //increment through the input string
        pos1++;
        pos2++;
    }
    
    //create answer struct and assign values
    answer ans;
    ans.RLE = result;
    ans.freq = frequencies;

    return ans;
}

//referenced from thread2 file in Canvas
void* rleThread(void *x_void_ptr) {
	struct arg *x_ptr = (struct arg *)x_void_ptr;
    x_ptr->RLE = runEncoder(x_ptr->s).RLE;
    x_ptr->freq = runEncoder(x_ptr->s).freq;
	return NULL;
}

int main(){
    vector<string>inputs;
    string input;
    
    //take in input and add to string vector
    while(cin >> input)
        inputs.push_back(input);

    int size = inputs.size();

    static  arg *x = new arg[size];
    pthread_t tid[size];

    //create a thread for each input string
    for(int i=0; i < size; i++){
        x[i].s = i;
		if(pthread_create(&tid[i], NULL, rleThread, &x[i])) {
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
    }

    for(int i = 0; i < size; i++){
        pthread_join(tid[i], NULL);
    }

    //run encoder function for each input line and output values
    for(int i=0; i < size; i++){
        cout << "Input string: " << inputs[i] << endl;
        answer output = runEncoder(inputs[i]);

        cout << "RLE String: " << output.RLE << endl;

        cout << "RLE Frequencies:";
        for(int j=0; j < output.freq.size(); j++){
            cout <<" "<< output.freq[j];
        }
        cout << endl;
    }
    return 0;
}