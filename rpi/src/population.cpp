#include "population.hpp"

#include <cstdint>
#include <iostream>

namespace audiogen {

Population::Population() {}
Population::Population(uint8_t n, Genome seed) {
    // make N genomes using the seed
    std::cout << "Making " << n << " genomes from " << std::endl;
    (void)seed;
}
Population::~Population() {}

}  // end audiogen
