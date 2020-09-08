#include <iostream>
#include <mpi.h>
#include <chrono>
#include "solution.cpp"

using namespace std;

int main() {
    MPI_Init(nullptr, nullptr);
    ofstream duration("duration.txt");
    auto start = std::chrono::high_resolution_clock::now();
    solution solver;
    solver.generateSystemOfEquation();
    solver.initializeSystem();
    for (int iteration = 0; iteration < 1000 && !solver.checkPrecision(); iteration++) {
        solver.computation();
    }
    solver.writeResultToFile();
    auto finish = std::chrono::high_resolution_clock::now();
    MPI_Finalize();
    duration << std::chrono::duration<double>(finish - start).count() << endl;
    duration.close();
    return solver.checkPrecision() ? 0 : -1;
}