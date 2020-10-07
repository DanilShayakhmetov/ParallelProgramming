#include "iostream"
#include <cmath>
#include <mpi.h>
#include <fstream>

using namespace std;

class solution {
private:
    int size = 10;
    double eps = 1;
    int offset;
    int partSize;
    double *A;
    double *b;
    double *x;
    double *prevX = nullptr;
    string const systemOfEquationsFile = "coefficients.txt";
    string const equationSystemSolutionFile = "results.txt";

public:
    const int mainProcessId = 0;
    int processNumber, processQuantity;

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

    void generateSystemOfEquation() {
        auto *systemItems = new double [size * (size + 1)];
        ofstream equationSystem(systemOfEquationsFile);
        ofstream solutionOfSystem(equationSystemSolutionFile);
        equationSystem << size << " " << size + 1 << endl;
        solutionOfSystem << size << 1 << endl;
        if (size >= 1) {
            for (int i = 0; i != size; ++i) {
                for (int j = 0; j != size + 1; ++j) {
                    if (i == j) {
                        systemItems[i * size + j] = 3 * size + rand() % (5 * size - 3 * size);
                    } else {
                        systemItems[i * size + j] = rand() % 3;
                    }
                    equationSystem << systemItems[i * size + j] << " ";
                }
                equationSystem << "\r\n";
                solutionOfSystem << rand() % 3 << "\r\n";

            }
        }
    }

    void readSystemFromFile() {
        ifstream equationSystem(systemOfEquationsFile);
        equationSystem >> size;
        b = new double[size];
        x = new double[size];
        A = new double[size * size];
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size + 1; ++j) {
                if (j == size) {
                    equationSystem >> b[i];
                } else {
                    equationSystem >> A[i * (size - 1) + j];
                }
            }
        }
        equationSystem.close();
    }

    void writeResultToFile() {
        if (this->isMainProcess()) {
            ofstream solutionOfSystem(equationSystemSolutionFile);
            for (auto i = 0; i < size; i++) {
                solutionOfSystem << x[i] << " ";
            }
            solutionOfSystem.close();
        }
    }

    void initializeSystem() {
        if (this->isMainProcess()) {
            readSystemFromFile();
        }

        MPI_Bcast(&size, 1, MPI_INT, mainProcessId, MPI_COMM_WORLD);
        MPI_Bcast(&eps, 1, MPI_DOUBLE, mainProcessId, MPI_COMM_WORLD);
        MPI_Bcast(x, size, MPI_DOUBLE, mainProcessId, MPI_COMM_WORLD);

        if (this->isMainProcess()) {
            int offset = 0;
            for (int process = 0; process < processQuantity; process++) {
                int partSize = size / processQuantity + (size % processQuantity > process);
                if (process == mainProcessId) {
                    this->offset = offset;
                    this->partSize = partSize;
                } else {
                    MPI_Send(&offset, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&partSize, 1, MPI_INT, process, 0, MPI_COMM_WORLD);

                    MPI_Send(&A[offset * size], partSize * size, MPI_DOUBLE, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&b[offset], partSize, MPI_DOUBLE, process, 0, MPI_COMM_WORLD);
                }
                offset += partSize;
            }
        } else {
            MPI_Status status;
            MPI_Recv(&offset, 1, MPI_INT, mainProcessId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&partSize, 1, MPI_INT, mainProcessId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            A = new double[size * partSize];
            MPI_Recv(A, size * partSize, MPI_DOUBLE, mainProcessId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            b = new double[partSize];
            MPI_Recv(b, size, MPI_DOUBLE, mainProcessId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }

    void computation() {
        if (prevX == nullptr) {
            prevX = new double[size];
        }
        for (int i = 0; i < size; i++) {
            prevX[i] = x[i];
        }

        for (int i = offset; i < offset + size; i++) {
            double sum = 0;
            for (int j = 0; j < size; j++) {
                if (i != j) {
                    sum += A[(i - offset) * size + j] * prevX[j];
                }
            }
            x[i] = (b[i - offset] - sum) / A[(i - offset) * size + i];
        }

        const int partSize = size / processQuantity;
        const int lastPartSize = size - (processQuantity - 1) * partSize ;
        for (int process = 0; process < processQuantity; process++) {
            const int offset = process * partSize;
            if (process < processQuantity - 1) {
                MPI_Bcast(&x[offset], partSize, MPI_DOUBLE, process, MPI_COMM_WORLD);
            } else {
                MPI_Bcast(&x[offset], lastPartSize, MPI_DOUBLE, process, MPI_COMM_WORLD);
            }
        }
    }

    bool checkPrecision() {
        if (prevX == nullptr) {
            return false;
        }
        for (int i = 0; i < size; i++) {
            if (abs(x[i] - prevX[i]) > eps) {
                return false;
            }
        }
        return true;
    }

private:
    bool isMainProcess() const {
        if (processNumber == mainProcessId) {
            return true;
        } else {
            return false;
        }
    };
};
