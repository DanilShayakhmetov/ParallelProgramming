#include <iostream>
#include <mpi.h>
#include <fstream>
#include <cmath>
#include <cstring>


using namespace std;

class QuickSort {

private:
    int arraySize = 1000;
    int *arraydata;
    string const dataFile = "dataset.txt";
    string const sortedDataFile = "sorted.txt";

public:
    const int mainProcessId = 0;
    int processNumber, processQuantity;


    static void swap(int *arr, int i, int j) {
        int tmp;
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }

    void quickSort(int *arr, int left, int right) {
        int i, last;
        if (left >= right)
            return;
        swap(arr, left, (left + right) / 2);
        last = left;
        for (i = left + 1; i <= right; i++)
            if (arr[i] < arr[left])
                swap(arr, ++last, i);
        swap(arr, left, last);
        quickSort(arr, left, last - 1);
        quickSort(arr, last + 1, right);
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

    void writeResult(const int len, const int *array) {
        ofstream dataset;
        dataset.open(sortedDataFile);
        dataset << len << endl;
        for (auto i = 0; i < len; i++)
            dataset << array[i] << " ";
        dataset.close();
    }

    void parallelQuickSort(const int size, const int rank, const int len, int *array, int *lengths) {
        auto iterations = log(size) / log(2);
        int *arrayPart = new int[len];
        int *buffer = new int[len];
        int *offsets = new int[size]{0};

        for (auto iteration = 0; iteration < iterations; iteration++) {
            for (auto i = 1; i < size; i++)
                offsets[i] = offsets[i - 1] + lengths[i - 1];

            int mySize;
            MPI_Scatter(lengths, 1, MPI_INT, &mySize, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Scatterv(array, lengths, offsets, MPI_INT, arrayPart, mySize, MPI_INT, 0, MPI_COMM_WORLD);

            int color = rank / pow(2, iterations - iteration);
            MPI_Comm MPI_LOCAL_COMMUNICATOR;
            MPI_Comm_split(MPI_COMM_WORLD, color, rank, &MPI_LOCAL_COMMUNICATOR);

            int localRank, localSize;
            MPI_Comm_rank(MPI_LOCAL_COMMUNICATOR, &localRank);
            MPI_Comm_size(MPI_LOCAL_COMMUNICATOR, &localSize);

            auto pivot = 0;
            if (localRank == 0 && mySize != 0)
                pivot = arrayPart[0 + rand() % lengths[rank]];
            MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_LOCAL_COMMUNICATOR);

            auto lessSize = 0;
            auto greaterSize = 0;
            auto lessBorder = 0;
            auto greaterBorder = lengths[rank] - 1;
            while (lessBorder <= greaterBorder) {
                if (arrayPart[lessBorder] <= pivot) {
                    lessBorder++;
                    lessSize++;
                } else if (arrayPart[greaterBorder] > pivot) {
                    greaterBorder--;
                    greaterSize++;
                } else {
                    auto tmp = arrayPart[lessBorder];
                    arrayPart[lessBorder] = arrayPart[greaterBorder];
                    arrayPart[greaterBorder] = tmp;
                }
            }

            auto rankFromLowerGroup = localRank < localSize / 2;
            auto sendTo = 0;
            auto recFrom = 0;
            auto bufferSize = 0;
            if (rankFromLowerGroup) {
                sendTo = recFrom = localRank + localSize / 2;
                MPI_Send(&greaterSize, 1, MPI_INT, sendTo, 0, MPI_LOCAL_COMMUNICATOR);
                MPI_Recv(&bufferSize, 1, MPI_INT, recFrom, 0, MPI_LOCAL_COMMUNICATOR, MPI_STATUS_IGNORE);
                MPI_Send(arrayPart + lessSize, greaterSize, MPI_INT, sendTo, 0, MPI_LOCAL_COMMUNICATOR);
                MPI_Recv(buffer, bufferSize, MPI_INT, recFrom, 0, MPI_LOCAL_COMMUNICATOR, MPI_STATUS_IGNORE);
            } else {
                sendTo = recFrom = localRank - localSize / 2;
                MPI_Recv(&bufferSize, 1, MPI_INT, recFrom, 0, MPI_LOCAL_COMMUNICATOR, MPI_STATUS_IGNORE);
                MPI_Send(&lessSize, 1, MPI_INT, sendTo, 0, MPI_LOCAL_COMMUNICATOR);
                MPI_Recv(buffer, bufferSize, MPI_INT, recFrom, 0, MPI_LOCAL_COMMUNICATOR, MPI_STATUS_IGNORE);
                MPI_Send(arrayPart, lessSize, MPI_INT, sendTo, 0, MPI_LOCAL_COMMUNICATOR);
            }

            if (rankFromLowerGroup) {
                memcpy(buffer + bufferSize, arrayPart, lessSize * sizeof(int));
                bufferSize += lessSize;
            } else {
                memcpy(buffer + bufferSize, arrayPart + lessSize, greaterSize * sizeof(int));
                bufferSize += greaterSize;
            }

            if (iteration == iterations - 1)
                quickSort(buffer, 0, bufferSize);

            MPI_Allgather(&bufferSize, 1, MPI_INT, lengths, 1, MPI_INT, MPI_COMM_WORLD);

            for (auto i = 1; i < size; i++)
                offsets[i] = offsets[i - 1] + lengths[i - 1];
            MPI_Gatherv(buffer, bufferSize, MPI_INT, array, lengths, offsets, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Comm_free(&MPI_LOCAL_COMMUNICATOR);
        }

        delete[] arrayPart;
        delete[] buffer;
        delete[] offsets;
    }

    int sort() {

        MPI_Comm_size(MPI_COMM_WORLD, &processQuantity);
        MPI_Comm_rank(MPI_COMM_WORLD, &processNumber);

        if (processNumber == 0) {
            readDataset();
        }

        MPI_Bcast(&arraySize, 1, MPI_LONG, 0, MPI_COMM_WORLD);

        int *lengths = new int[processQuantity];
        for (auto i = 0; i < processQuantity; i++)
            lengths[i] = arraySize / processQuantity;
        for (auto i = 0; i < arraySize % processQuantity; i++)
            lengths[i]++;

        if (processQuantity == 1)
            quickSort(arraydata, 0, arraySize);
        else
            parallelQuickSort(processQuantity, processNumber, arraySize, arraydata, lengths);

        if (processNumber == mainProcessId) {
            writeResult(arraySize, arraydata);
        }

        delete[] arraydata;
        delete[] lengths;
    }

};