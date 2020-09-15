#include <iostream>
#include <mpi.h>
#include "QuickSort.cpp"

using namespace std;

int main() {
    MPI_Init(nullptr, nullptr);
    QuickSort sorter;
    sorter.prepareDataset();
    sorter.readDataset();
    sorter.start();
    MPI_Finalize();
}
