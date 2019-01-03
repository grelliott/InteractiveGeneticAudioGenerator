#pragma once

#include <set>

#include "genome.hpp"

namespace audiogen {

    class Population {
        std::set<Genome> mGenomes;
     public:
        Population();
        Population(uint8_t n, Genome seed);
        ~Population();

        //Population fittest(const uint8_t n, const FitnessAlgorithm algorithm);
    };
}  // end audiogen
