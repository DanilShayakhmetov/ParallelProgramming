#include <iostream>
#include <chrono>
#include "matrix.cpp"
#include "multiplier.cpp"

using namespace std;


double multiply(int a, int b, int c, int type) {
 Multiplier * multiplier;
 auto matrixA = new Matrix(a,c);
 auto matrixB = new Matrix(c,b);
 matrixA->fillRandom();
 matrixB->fillRandom();
    switch (type) {
        case 0:
            multiplier = new Multiplier(matrixA, matrixB);
            break;
        case 1:
            multiplier = new StaticScheduleMultiplier(matrixA, matrixB);
            break;
        case 2:
            multiplier = new DynamicScheduleMultiplier(matrixA, matrixB);
            break;
        case 3:
            multiplier = new GuidedScheduleMultiplier(matrixA, matrixB);
            break;
    }
    auto start = chrono::high_resolution_clock::now();
    multiplier->multiply();
    auto finish = chrono::high_resolution_clock::now();
    return chrono::duration<double>(finish - start).count();
}

int main() {
    int rows = 150;
    int columns = 150;
    int type = 0;
    int executeCount = 10;
    double averageTime = 0;

    for (int i = 0; i < executeCount; ++i) {
        averageTime += multiply(rows, columns, 150 ,type);
    }

    averageTime = averageTime / executeCount;
    cout << averageTime;

    return 0;
}