#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <pthread.h>
#include <set>
#include <cmath>

using namespace std;

struct argMapper {
    vector<vector<vector<int>>>* mapperMatrix;
    stack<string>* inputFiles;
    pthread_mutex_t* mutex;
    pthread_barrier_t* barrier;
    int indexMapper;
    int nrReducers;
};

struct argReducer {
    int indexReducer;
    vector<set<int>>* result;
    vector<vector<vector<int>>>* mapperMatrix;
    pthread_barrier_t* barrier;
    int nrMappers;
};

bool isSquareNumber(size_t number, size_t exp) {
    if (number == 1) { return true; }
    if (number == 0) { return false; }

    size_t left = 1;
    size_t right = number / 2;

    // avoid overflows
    while (pow(right, exp) > static_cast<double>(INT_FAST32_MAX)) {
        right /= 2;
    }

    while (left < right) {
        size_t mid = (left + right) / 2;
        size_t midPow = pow(mid, exp);

        if (midPow < number) { 
            left = mid + 1;
        } else if (midPow > number) { 
            right = mid - 1;
        } else {
            return true;
        }
    }

    return pow(left, exp) == number;
}

void *funcMapper(void *arg) {
    argMapper argument = *((argMapper*)arg);
    string file;

    while (true) {
        pthread_mutex_lock(argument.mutex);
        if (!argument.inputFiles->empty()) {
            file = argument.inputFiles->top();
            argument.inputFiles->pop();
            pthread_mutex_unlock(argument.mutex);

            ifstream inFile(file);
            int numbers, nr;
            inFile >> numbers;
            for (int i = 0; i < numbers; i++) {
                inFile >> nr;
                for (int j = 0; j < argument.nrReducers; j++) {
                    if (isSquareNumber(nr, j + 2)) {
                        (*argument.mapperMatrix)
                        [argument.indexMapper][j].push_back(nr);
                    }
                }
            }
        } else {
            pthread_mutex_unlock(argument.mutex);
            break;
        }
    }
    
    pthread_barrier_wait(argument.barrier);

    pthread_exit(NULL);
}

void *funcReducer(void *arg) {
    argReducer argument = *((argReducer*)arg);
    pthread_barrier_wait(argument.barrier);


    for (int i = 0; i < argument.nrMappers; i++) {
        for (int j = 0; j < (int)(*(argument.mapperMatrix))
                [i][argument.indexReducer].size(); j++)
        {
            (*(argument.result))[argument.indexReducer]
            .insert((*(argument.mapperMatrix))[i][argument.indexReducer][j]);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("Parametrii insuficienti\n");
        exit(-1);
    }

    // read command line parameters
    int nrMappers = atoi(argv[1]);
    int nrReducers = atoi(argv[2]);
    string inputFile = argv[3];

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, nrMappers + nrReducers);

    // create the argument structure for mapper function
    vector<vector<vector<int>>>* mapperMatrix;
    mapperMatrix = new vector<vector<vector<int>>>(
        nrMappers,vector<vector<int>>(nrReducers, vector<int>()));

    stack<string>* inputFiles = new stack<string>();

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    argMapper argMap;
    argMap.mapperMatrix = mapperMatrix;
    argMap.inputFiles = inputFiles;
    argMap.mutex = &mutex;
    argMap.barrier = &barrier;
    argMap.nrReducers = nrReducers;

    // fill the stack with all the input files from argv[3] (text.txt)
    ifstream inFile(inputFile);
    int nrInputFiles;
    inFile >> nrInputFiles;
    string file;
    for (int i = 0; i < nrInputFiles; i++) {
        inFile >> file;
        inputFiles->push(file);
    }

    // create the mappers and reducers
    pthread_t mappers[nrMappers];
    pthread_t reducers[nrReducers];
    void *status;
    int resp, id;

    argMapper argMapperArray[nrMappers];

    for (id = 0; id < nrMappers; id++) {
        argMap.indexMapper = id;
        argMapperArray[id] = argMap;
        resp = pthread_create(&mappers[id], NULL,
                              funcMapper, &(argMapperArray[id]));

        if (resp) {
            printf("Eroare la crearea mapper-ului %d\n", id);
	  		exit(-1);
        }
    }

    // create the argument structer for reducer function
    vector<set<int>>* reducerResult;
    reducerResult = new vector<set<int>>(nrReducers);

    argReducer argReduce;
    argReduce.result = reducerResult;
    argReduce.mapperMatrix = mapperMatrix;
    argReduce.barrier = &barrier;
    argReduce.nrMappers = nrMappers;

    argReducer argReducerArray[nrReducers];
    
    for (id = 0; id < nrReducers; id++) {
        argReduce.indexReducer = id;
        argReducerArray[id] = argReduce;
        resp = pthread_create(&reducers[id], NULL,
                                funcReducer, &(argReducerArray[id]));

        if (resp) {
            printf("Eroare la crearea reducer-ului %d\n", id);
	  		exit(-1);
        }
    }

    for (id = 0; id < nrMappers; id++) {
		resp = pthread_join(mappers[id], &status);

		if (resp) {
	  		printf("Eroare la asteptarea mapper-ului %d\n", id);
	  		exit(-1);
		}
  	}
    for (id = 0; id < nrReducers; id++) {
		resp = pthread_join(reducers[id], &status);

		if (resp) {
	  		printf("Eroare la asteptarea reducer-ului %d\n", id);
	  		exit(-1);
		}
  	}

    // print the results in output files
    for (int i = 0; i < nrReducers; i++) {
        ofstream file("out" + std::to_string(i + 2) + ".txt");
        file << (*argReduce.result)[i].size();
        file.close();
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
  	pthread_exit(NULL);

    return 0;
}