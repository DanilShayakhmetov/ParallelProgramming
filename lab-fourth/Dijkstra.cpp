#pragma once

#include "Graph.cpp"

using namespace std;

const int inf = 888888888;

class Dijkstra {
public:
    Graph *graph;

    bool *visited;
    int *distance;
    int source;
    int n;

    Dijkstra(Graph *graph, int source) : graph(graph), source(source), n(graph->nodesCount) {
        visited = new bool[n];
        distance = new int[n];
        for (int i = 0; i < n; ++i) {
            visited[i] = false;
            distance[i] = inf;
        }
    }

    int *execute(int type) {
        visited[source] = true;
        distance[source] = 0;
#pragma omp parallel for schedule(static) default(shared)
        for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
            if (vertex == source) {
                continue;
            }
            distance[vertex] = graph->getDistance(source, vertex);
        }
        for (int i = 0; i < graph->nodesCount - 1; ++i) {
            int currentVertex = findMinDistance();
            visited[currentVertex] = true;
            distanceCalculation(type, currentVertex);
        }
        return distance;
    }

private:
    void distanceCalculation(int type, int currentVertex) const {
        if (type == 0) {
            for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
                if (visited[vertex]) {
                    continue;
                }
                distance[vertex] = min(distance[vertex],
                                       distance[currentVertex] + graph->getDistance(currentVertex, vertex));
            }
        } else if (type == 1) {
#pragma omp parallel for schedule(static) default(shared)
            for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
                if (visited[vertex]) {
                    continue;
                }
                distance[vertex] = min(distance[vertex],
                                       distance[currentVertex] + graph->getDistance(currentVertex, vertex));
            }
        } else if (type == 2) {
#pragma omp parallel for schedule(dynamic) default(shared)
            for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
                if (visited[vertex]) {
                    continue;
                }
                distance[vertex] = min(distance[vertex],
                                       distance[currentVertex] + graph->getDistance(currentVertex, vertex));
            }
        } else if (type == 3) {
#pragma omp parallel for schedule(guided) default(shared)
            for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
                if (visited[vertex]) {
                    continue;
                }
                distance[vertex] = min(distance[vertex], distance[currentVertex] + graph->getDistance(currentVertex, vertex));
            }
        }
    }

    int findMinDistance() const {
        int minDistance = inf, minVertex;
        for (int vertex = 0; vertex < graph->nodesCount; ++vertex) {
            if (visited[vertex]) {
                continue;
            }
            if (distance[vertex] < minDistance) {
                minDistance = distance[vertex];
                minVertex = vertex;
            }
        }
        return minVertex;
    }
};
