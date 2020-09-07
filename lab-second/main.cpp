#include <iostream>
#include <cmath>
#include <mpi.h>
#include <fstream>
#include "solution.cpp"


using namespace std;

int main(int argc, char **argv) {
    MPI_Init(nullptr, nullptr);

    solution solution;
    solution.solver();
    solution.writeResults();

    MPI_Finalize();

}

