#pragma once

#include <iostream>
#include "Graph.cpp"
#include "Dijkstra.cpp"
#include <chrono>

using namespace std;

float duration(int type) {
    auto graph = new Graph();
    auto dijkstra = new Dijkstra(graph, 0);

    auto start = std::chrono::high_resolution_clock::now();
    dijkstra->execute(type);
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float>(finish - start).count();
    return duration;
}

int main() {

    float averageTime = 0;
    int const executeCount = 10;
    int const type = 1;

    for (int i = 0; i < executeCount; ++i) {
        averageTime += duration(type);
    }

    averageTime = averageTime / executeCount;
    cout << averageTime;
    return 0;
}