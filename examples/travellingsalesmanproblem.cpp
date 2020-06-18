
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "entities/solution.h"
#include "examples/combinatorial/indirect/tsp.h"
#include "solvers/differentialevolution.h"
#include "solvers/geneticalgorithm.h"
#include "solvers/greywolfoptimizer.h"
#include "solvers/iteratedlocalseach.h"
#include "solvers/parameters.h"
#include "solvers/particleswarmoptimization.h"
#include "util/timer.h"
#include "util/tspreader.h"
#include "util/util.h"

void travellingSalesmanProblemExample();

int main(int argc, char *argv[])
{
    travellingSalesmanProblemExample();
    exit(EXIT_SUCCESS);
}

// Example using Iterated Local Search or Differential Evolution to solve the travelling salesman problem
void travellingSalesmanProblemExample()
{
    const std::vector<std::vector<double>> distMatrix = reader::readTspInstance(reader::TspInstance::A280);
    const std::shared_ptr<TravellingSalesmanProblem> tsp =
        make_shared<TravellingSalesmanProblem>(distMatrix.size(), distMatrix, OptimizationStrategy::MINIMIZE, RepresentationType::INDIRECT);

    // Check solvers/parameter.h file to understand the definition of the parameters
    DifferentialEvolutionParameters parameters = {8, 0.0, 0.5, std::log10(tsp->getDimension()), false};
    DifferentialEvolution<encoding> de(parameters, tsp);
    de.setRunningTime(600);
    de.solve();
}
