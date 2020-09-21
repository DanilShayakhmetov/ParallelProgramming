#pragma once

#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;

class Graph {
public:
    const int INFINITE = 10000;
    int nodesCount = 50;
    string  const filename = "Graph" + to_string(nodesCount);
    int *graph = new int [nodesCount * nodesCount];

    Graph(){
        fillRandom();
        writeToFile();
        readFromFile();
    }

    bool writeToFile() const {
        if (nodesCount == 0) {
            return false;
        }
        ofstream file(filename);
        file << nodesCount << endl;
        for (size_t i = 0; i != nodesCount; ++i) {
            for (size_t j = 0; j != nodesCount; ++j) {
                file << graph[i * nodesCount + j] << " ";
            }
            file << endl;
        }
        file.close();
        return true;
    }

    bool readFromFile() {
        ifstream file(filename);
        file >> nodesCount;
        graph = new int [nodesCount * nodesCount];
        for (size_t i = 0; i < nodesCount; ++i) {
            for (size_t j = 0; j < nodesCount; ++j) {
                file >> graph[i * nodesCount + j];
            }
        }
        file.close();
    }

    int getDistance(int source, int node) const {
        return graph[source * nodesCount + node];
    }

    void fillRandom() const {
        if (nodesCount > 0) {
            for (int i = 0; i < nodesCount; ++i) {
                for (int j = 0; j < nodesCount; ++j) {
                    if (i == j) {
                        graph[i * nodesCount + j] = 0;
                        continue;
                    }
                    if (rand() % 2) {
                        graph[i * nodesCount + j] = rand() % 10;
                    } else {
                        graph[i * nodesCount + j] = INFINITE;
                    }
                }
            }
        }
    }
};
