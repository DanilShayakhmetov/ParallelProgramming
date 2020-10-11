#include <iostream>
#include <mpi.h>
#include <fstream>

using namespace std;

class QuickSort {

private:
    int arraySize = 1000;
    int *arraydata;
    int *sortedPart;
    int *other;
    int partSize;
    int step;
    string const dataFile = "dataset.txt";
    string const sortedDataFile = "sorted.txt";
    MPI_Status status;

public:
    const int mainProcessId = 0;
    int processNumber, processQuantity;


    QuickSort() {
        MPI_Comm_size(MPI_COMM_WORLD, &processQuantity);
        MPI_Comm_rank(MPI_COMM_WORLD, &processNumber);
    }

    int *merge(int *leftPart, int leftPartSize, int *rightPart, int rightPartSize) {
        int i, j, k;
        int *result;

        result = new int[leftPartSize + rightPartSize];

        i = 0;
        j = 0;
        k = 0;
        while (i < leftPartSize && j < rightPartSize)
            if (leftPart[i] < rightPart[j]) {
                result[k] = leftPart[i];
                i++;
                k++;
            } else {
                result[k] = rightPart[j];
                j++;
                k++;
            }
        if (i == leftPartSize)
            while (j < rightPartSize) {
                result[k] = rightPart[j];
                j++;
                k++;
            }
        else
            while (i < leftPartSize) {
                result[k] = leftPart[i];
                i++;
                k++;
            }
        return result;
    }

    void swapParts(int *arr, int i, int j) {
        int tmp;
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }

    void quickSort(int *arr, int left, int right) {
        int i, last;
        if (left >= right)
            return;
        swapParts(arr, left, (left + right) / 2);
        last = left;
        for (i = left + 1; i <= right; i++)
            if (arr[i] < arr[left])
                swapParts(arr, ++last, i);
        swapParts(arr, left, last);
        quickSort(arr, left, last - 1);
        quickSort(arr, last + 1, right);
    }

    void sort() {
        if (processNumber == mainProcessId) {
            readDataset();
            partSize = arraySize / processQuantity;
            MPI_Bcast(&partSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
            sortedPart = new int[partSize];
            MPI_Scatter(arraydata, partSize, MPI_INT, sortedPart, partSize, MPI_INT, 0, MPI_COMM_WORLD);
            quickSort(sortedPart, 0, partSize - 1);
        } else {
            MPI_Bcast(&partSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
            sortedPart = new int[partSize];
            MPI_Scatter(arraydata, partSize, MPI_INT, sortedPart, partSize, MPI_INT, 0, MPI_COMM_WORLD);
            quickSort(sortedPart, 0, partSize - 1);
        }

        step = 1;
        while (step < processQuantity) {
            if (processNumber % (2 * step) == 0) {
                if (processNumber + step < processQuantity) {
                    MPI_Recv(&arraySize, 1, MPI_INT, processNumber + step, 0, MPI_COMM_WORLD, &status);
                    other = new int[arraySize];
                    MPI_Recv(other, arraySize, MPI_INT, processNumber + step, 0, MPI_COMM_WORLD, &status);
                    sortedPart = merge(sortedPart, partSize, other, arraySize);
                    partSize = partSize + arraySize;
                }
            } else {
                int near = processNumber - step;
                MPI_Send(&partSize, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
                MPI_Send(sortedPart, partSize, MPI_INT, near, 0, MPI_COMM_WORLD);
                break;
            }
            step = step * 2;
        }

        if (processNumber == mainProcessId) {
            writeResult(sortedPart);
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
        arraydata = new int[size];
        for (int i = 0; i < size; ++i) {
            dataset >> arraydata[i];
        }
        dataset.close();
    }

    void writeResult(int *sortedArray) {
        ofstream dataset(sortedDataFile);
        int size = sizeof(sortedArray);
        dataset << size << endl;
        for (auto i = 0; i < size; i++) {
            dataset << sortedArray[i] << " ";
        }
        dataset.close();
    }
};