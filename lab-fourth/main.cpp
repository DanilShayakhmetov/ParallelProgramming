#pragma once

#include <iostream>
#include "Graph.cpp"
#include "Dijkstra.cpp"
#include <chrono>

using namespace std;

double duration(int type) {
    auto graph = new Graph();
    auto dijkstra = new Dijkstra(graph, 0);

    auto start = std::chrono::high_resolution_clock::now();
    dijkstra->execute(type);
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(finish - start).count();
    return duration;
}

int main() {
    int type = 0;

    int executeCount = 10;
    double averageTime = 0;

    for (int i = 0; i < executeCount; ++i) {
        averageTime += duration(type);
    }

    averageTime = averageTime / executeCount;
    cout << averageTime;
    return 0;
}