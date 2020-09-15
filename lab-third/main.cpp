#include <iostream>
#include <mpi.h>
#include <chrono>
#include "QuickSort.cpp"

using namespace std;

int main() {
    MPI_Init(nullptr, nullptr);
    QuickSort sorter;
    sorter.prepareDataset();
    sorter.readDataset();
    auto start = chrono::high_resolution_clock::now();
    sorter.start();
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration<double>(finish - start).count() << endl;
    MPI_Finalize();
}
