#include <iostream>
#include <mpi.h>
#include <fstream>

using namespace std;

class QuickSort {

private:
    int arraySize = 100000;
    long pivot = 0;
    int partSize;
    long *arraydata;
    long *arrayPart;
    string const dataFile = "dataset.txt";
    string const sortedDataFile = "sorted.txt";

public:
    const int mainProcessId = 0;
    int processNumber, processQuantity;



    QuickSort() {
        MPI_Comm_size(MPI_COMM_WORLD, &processQuantity);
        MPI_Comm_rank(MPI_COMM_WORLD, &processNumber);
    }

    void start(){
        if (processNumber == mainProcessId) {
            readDataset();
            for (int proc = 0; proc < processQuantity; ++proc) {
                auto part = getPart(proc);
//                MPI_Request request;
//                MPI_Isend(part, partSize, MPI_INT, proc, 0, MPI_COMM_WORLD, &request);
            }
        }
//
//        arrayPart = acceptData(mainProcessId);

//        while (true) {
//            if (processQuantity == 1) {
//                sort(arrayPart, 0, partSize);
//                break;
//            }
//
//            if (processNumber == mainProcessId) {
//                pivot = getPivot();
//            }
//            MPI_Bcast(&pivot, 1, MPI_INT, mainProcessId, MPI_COMM_WORLD);
//            sort(arrayPart, 0, partSize);
//            swapParts();
//            separateProcess();
//        }
//
//        if (processNumber == mainProcessId) {
//            writeResult(joinParts());
//        } else {
//            joinParts();
//        }
    }

    long* getPart(int procId) {
        partSize = arraySize / processQuantity;
        int from = procId * partSize;
        if (procId == processQuantity - 1) {
            partSize = arraySize - from;
        }

        return &arraydata[from];
    }

    long* acceptData(int from) {
        int size;
        int *tmp = new int[arraySize];
        MPI_Status status;
        MPI_Recv(tmp, arraySize, MPI_INT, from, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &size);
        partSize = size;
        return &arraydata[from];
    }

    long getPivot() {
        long left = arrayPart[0];
        long right = arrayPart[partSize - 1];
        long middle = arrayPart[partSize / 2];
        if (left <= middle && middle <= right) {
            return middle;
        } else if (middle <= left && left <= right) {
            return left;
        } else {
            return right;
        }
    }

    void swapParts() {
        long *leftPart, *rightPart;
        int adjacentProcess = getAdjacentProcess();

        separateArray(*&leftPart, *&rightPart);
        int size;
        MPI_Request request;
        if (this->rightPart()) {
            size = sizeof(leftPart);
            MPI_Isend(leftPart, size, MPI_INT, adjacentProcess, 0, MPI_COMM_WORLD, &request);
        } else {
            size = sizeof(rightPart);
            MPI_Isend(rightPart, size, MPI_INT, adjacentProcess, 0, MPI_COMM_WORLD, &request);
        }

        auto acceptedData = acceptData(adjacentProcess);
        if (this->rightPart()) {
            arrayPart = unitParts(rightPart, acceptedData);
        } else {
            arrayPart = unitParts(leftPart, acceptedData);
        }
    }

    long *joinParts() {
        MPI_Request request;
        MPI_Isend(arrayPart, partSize, MPI_INT, mainProcessId, 0, MPI_COMM_WORLD, &request);
        long *result = nullptr;
        if (processNumber == mainProcessId) {
            for (int process = 0; process < processQuantity; ++process) {
                auto *array = acceptData(process);

                if (result == nullptr) {
                    result = array;
                } else {
                    result = unitParts(result, array);
                }
            }
            return result;
        } else {
            return result;
        }
    }

    void separateProcess() {
        int newGroup = rightPart() ? 1 : 0;
        MPI_Comm communicator;
        MPI_Comm_split(MPI_COMM_WORLD, newGroup, 0, &communicator);
    }

    void separateArray(long *leftPart, long *rightPart) {
        int left = 0;
        int right = partSize - 1;
        while (true) {
            do {
                ++left;
            } while (arrayPart[left] < pivot);
            do {
                --right;
            } while (arrayPart[right] >= pivot);
            if (left >= right) {
                break;
            }
            swap(arrayPart[left], arrayPart[right]);
        }
        leftPart = new long [left];
        rightPart = new long [right];
        for (int i = 0; i < partSize; i++) {
            if (i < left) {
                leftPart[i] = arrayPart[i];
            } else {
                rightPart[i] = arrayPart[i];
            }
        }
    }

    bool rightPart() const {
        return processNumber >= processQuantity / 2;
    }

    int getAdjacentProcess() const {
        if (rightPart()) {
            return processNumber - (processQuantity + 1) / 2;
        }
        return processNumber + (processQuantity + 1) / 2;
    }

    static void split(long* arr, int left, int right, int &t){
        long x = arr[left];
        long tmp;
        t = left;
        for(int i = left + 1; i <= right; i++) {
            if(arr[i] < x) {
                t++;
                tmp = arr[t];
                arr[t] = arr[i];
                arr[i] = tmp;
            }
        }
        tmp = arr[left];
        arr[left] = arr[t];
        arr[t] = tmp;
    }

    void sort(long* arr, int left, int right) {
        if(left < right) {
            int t = 0;
            split(arr, left, right, t);
            sort(arr, left, t);
            sort(arr, t + 1, right);
        }
    }

    long* unitParts (long *left, long *right) {
        int lSize = sizeof(left);
        int rSize = sizeof(right);
        int size = lSize + rSize;
        arrayPart = new long [size];
        for (int i = 0; i < lSize; ++i) {
            arrayPart[i] = left[i];
        }
        for (int i = lSize; i < size; ++i) {
            arrayPart[i] = right[i - lSize];
        }
    }

    void prepareDataset() {
        ofstream dataset(dataFile);
        dataset << arraySize << endl;
        for (int i = 0; i < arraySize; ++i) {
            dataset << rand() % 100 << " ";
        }
        dataset.close();
    }

    void readDataset() {
        ifstream dataset(dataFile);
        int size;
        dataset >> size;
        arraydata = new long[size];
        for (int i = 0; i < size; ++i) {
            dataset >> arraydata[i];
        }
        dataset.close();
    }

    void writeResult(long *sortedArray) {
        ofstream dataset(sortedDataFile);
        int size = sizeof(sortedArray);
        dataset << size << endl;
        for (auto i = 0; i < size; i++) {
            dataset << sortedArray[i] << " ";
        }
        dataset.close();
    }
};