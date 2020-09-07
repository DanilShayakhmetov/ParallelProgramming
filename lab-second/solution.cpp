#include <iostream>
#include <cmath>
#include <mpi.h>
#include <fstream>


using namespace std;

class solution {
private:
    int systemSize{};
    double *A{};
    double *b{};
    double eps{};
    double *x{};
    double *prevX = nullptr;
    int offset{};
    int partSize{};
    string const inputFile = "inputFile.txt";
    string const outputFile = "outputFile.txt";

public:
    const int mainProcessId = 0;
    int processNumber{}, processQuantity{};

    solution() {
        MPI_Comm_rank(MPI_COMM_WORLD, &processNumber);
        MPI_Comm_size(MPI_COMM_WORLD, &processQuantity);
    }

    ~solution() {
        delete[] A;
        delete[] b;
        delete[] x;
        delete[] prevX;
    }

    double *solver() {
        getParams();

        MPI_Bcast(&systemSize, 1, MPI_INT, mainProcessId, MPI_COMM_WORLD);
        MPI_Bcast(&eps, 1, MPI_INT, mainProcessId, MPI_COMM_WORLD);

        if (this->isMainProcess()) {
            x = new double[systemSize];
        }

        MPI_Bcast(x, systemSize, MPI_DOUBLE, mainProcessId, MPI_COMM_WORLD);

        if (this->isMainProcess()) {
            int currentOffset = 0;
            for (int process = 0; process < processQuantity; process++) {
                int currentPart = systemSize / processQuantity + (systemSize % processQuantity > process);
                if (process == mainProcessId) {
                    this->offset = currentOffset;
                    this->partSize = currentPart;
                } else {
                    MPI_Send(&currentOffset, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&currentPart, 1, MPI_INT, process, 0, MPI_COMM_WORLD);

                    MPI_Send(&A[currentOffset * systemSize], currentPart * systemSize, MPI_DOUBLE, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&b[currentOffset], currentPart, MPI_DOUBLE, process, 0, MPI_COMM_WORLD);
                }
                currentOffset += currentPart;
            }
        }
        bool accuracyAchieved = false;
        for (int iteration = 0; iteration < 1000 && !accuracyAchieved; iteration++) {
            if (prevX == nullptr) {
                accuracyAchieved = false;
            }
            for (int i = 0; i < systemSize; i++) {
                if (abs(x[i] - prevX[i]) > eps) {
                    accuracyAchieved = false;
                }
            }
            accuracyAchieved = true;
            if (prevX == nullptr) {
                prevX = new double[systemSize];
            }
            for (int i = 0; i < systemSize; i++) {
                prevX[i] = x[i];
            }

            for (int i = offset; i < offset + partSize; i++) {
                double sum = 0;
                for (int j = 0; j < systemSize; j++) {
                    if (i != j) {
                        sum += A[(i - offset) * systemSize + j] * prevX[j];
                    }
                }
                x[i] = (b[i - offset] - sum) / A[(i - offset) * systemSize + i];
            }

            int part = systemSize / processQuantity;
            for (int process = 0; process < processQuantity; process++) {
                int const displacement = process * part;
                int const size = (process < processQuantity - 1) ? part : systemSize - displacement;
                MPI_Bcast(&x[displacement], size, MPI_DOUBLE, process, MPI_COMM_WORLD);
            }


        }
    }

    void writeResults() {
        if (this->isMainProcess()) {
            ofstream stream(outputFile);
            for (auto i = 0; i < systemSize; i++) {
                stream << x[i] << " ";
            }
            stream.close();
        }
    }

private:
    void getParams() {
        if (this->isMainProcess()) {
            ifstream stream(inputFile);
            stream >> systemSize >> eps;
            x = new double[systemSize];
            for (auto i = 0; i < systemSize; i++) {
                stream >> x[i];
            }
            b = new double[systemSize];
            for (auto i = 0; i < systemSize; i++) {
                stream >> b[i];
            }
            A = new double[systemSize * systemSize];
            for (auto i = 0; i < systemSize; i++) {
                for (auto j = 0; j < systemSize; j++) {
                    stream >> A[i * systemSize + j];
                }
            }
            stream.close();
        }
    }
    bool isMainProcess() {
        if (processNumber == mainProcessId) {
            return true;
        } else {
            return false;
        }
    };
};

