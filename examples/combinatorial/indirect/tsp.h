#ifndef TSP_H
#define TSP_H

#include <algorithm>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>

#include "../../../entities/problem.h"

typedef double encoding;

namespace tsp
{ // TSP problem related structures
std::vector<std::pair<double, double>> rawNodes;
std::vector<std::vector<double>> distanceMatrix;

double euclideanDistance(double x1, double y1, double x2, double y2) { return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)); }

void calculateDistances(std::vector<std::pair<double, double>> rawNodes, int dimension)
{
    tsp::rawNodes = rawNodes;
    distanceMatrix = std::vector<std::vector<double>>(dimension, std::vector<double>(dimension, 0));
    for (size_t i = 0; i < dimension; i++)
        for (size_t j = 0; j < dimension; j++)
            distanceMatrix[i][j] = round(euclideanDistance(rawNodes[i].first, rawNodes[i].second, rawNodes[j].first, rawNodes[j].second));
}

// Encodes permutation into continuous domain
// The lowest the index, the lowest is the continuous value
void encode(const std::vector<int> &permutation, std::vector<double> &decisionVariables)
{
    int dimension = permutation.size();
    double share = 1.0 / dimension, acc = std::nextafter(0.0, 0.1);
    for (size_t k = 0; k < dimension; k++) {
        const int index = permutation[k];
        decisionVariables[index] = acc;
        acc += share;
    }
}
} // namespace tsp

class TSPSolution : public Solution<encoding>
{
public:
    TSPSolution(int dimension, std::vector<encoding> &decVar) : Solution(decVar)
    {
        this->dimension = dimension;
        this->fitness = 0;
        this->createPermutation(decVar);
        this->calculateFitness();
    }

    // 2-opt Swap mechanism
    // Reverse the direction of the path between indexes start and end
    void twoOptSwap(const int start, const int end, std::vector<int> &newPermutation)
    {
        newPermutation = std::vector<int>(dimension);
        for (int i = 0; i < start; i++)
            newPermutation[i] = permutation[i];
        int counter = start;
        for (int i = end; i >= start; i--) {
            newPermutation[counter] = permutation[i];
            counter++;
        }
        for (int i = end + 1; i < permutation.size(); i++)
            newPermutation[i] = permutation[i];
    }

    // This 2-opt neighborhood function
    void localSearch() override
    {
        std::vector<int> newPermutation(dimension);
        std::vector<double> newDecisionVariables(dimension);
        std::vector<double> bestDecisionVariables;
        std::shared_ptr<TSPSolution> bestNeighbor = NULL;
        while (true) {
            // Check every neighbor solution obtaining by swapping elements of the permutation
            for (size_t i = 0; i < dimension - 1; i++) {
                for (size_t j = i + 1; j < dimension; j++) {
                    // Define Neighbor solution by inverting the path between two points
                    twoOptSwap(i, j, newPermutation);
                    // Decision variables values of the neighbor solution
                    tsp::encode(newPermutation, newDecisionVariables);
                    // Construct neighbor solution
                    std::shared_ptr<TSPSolution> neighbor = std::make_shared<TSPSolution>(this->dimension, newDecisionVariables);
                    // Check if the current neighbor is the best neighbor
                    if (bestNeighbor == NULL || bestNeighbor->fitness > neighbor->fitness) {
                        bestNeighbor = neighbor;
                        bestDecisionVariables = std::vector<double>(newDecisionVariables);
                    }
                }
            }

            // Replace current solution if the best neighbor is better, else stop local search
            if (bestNeighbor != NULL && bestNeighbor->fitness < this->fitness) {
                std::shared_ptr<TSPSolution> best = std::make_shared<TSPSolution>(this->dimension, bestDecisionVariables);
                this->fitness = best->fitness;
                this->permutation = best->permutation;
                this->decisionVariables = best->decisionVariables;
                break;
            } else
                break;
        }
    }

    void print() override
    {
        std::cout << "Path: { ";
        for (size_t i = 0; i < dimension; i++) {
            int index = permutation[i];
            if (i + 1 < dimension)
                std::cout << index + 1 << "->";
            else
                std::cout << index + 1 << "\n";
        }
    }

    std::vector<int> getPermutation() const { return permutation; }

protected:
    int dimension;
    std::vector<int> permutation; // stores the permutation of the nodes, i.e., the order in which the nodes are visited

    void createPermutation(const std::vector<encoding> &decisionVariables)
    {
        permutation = std::vector<int>(dimension);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::sort(permutation.begin(), permutation.end(),
              [&](int pos1, int pos2) { return std::tie(decisionVariables[pos1], pos1) < std::tie(decisionVariables[pos2], pos2); });
    }

    void calculateFitness()
    {
        this->fitness = 0;
        // Sum of the edges' weight of the path
        for (size_t i = 0; i < dimension - 1; i++) {
            int index1 = permutation[i];
            int index2 = permutation[i + 1];
            this->fitness += tsp::distanceMatrix[index1][index2];
        }

        // Add the weight from the last to the first
        int index1 = permutation[dimension - 1];
        int index2 = permutation[0];
        this->fitness += tsp::distanceMatrix[index1][index2];
    }
};

class TravellingSalesmanProblem : public Problem<encoding>
{
public:
    TravellingSalesmanProblem(int dimension, const std::vector<std::pair<double, double>> rawNodes, OptimizationStrategy strategy, RepresentationType repType)
        : Problem(strategy, repType)
    {
        if (dimension == 0) {
            std::cerr << "Zero nodes were given as input\n";
            exit(1);
        }

        this->dimension = dimension;
        this->lb = std::vector<encoding>(dimension);
        this->ub = std::vector<encoding>(dimension);

        for (size_t i = 0; i < this->dimension; i++) {
            lb[i] = 0;
            ub[i] = nextafter(1.0, 0.0);
        }

        tsp::calculateDistances(rawNodes, dimension);
    }

    std::shared_ptr<Solution<double>> construct(std::vector<encoding> &decisionVariables) override
    {
        std::shared_ptr<TSPSolution> solution = std::make_shared<TSPSolution>(this->dimension, decisionVariables);
        // solution->localSearch();
        numbTriedSolution++;
        return solution;
    }
};

#endif // TSP_H
