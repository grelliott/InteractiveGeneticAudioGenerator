#include "genome.hpp"

#include <iostream>
#include <cassert>
#include <map>
#include <string>

namespace audiogen {

Genome::Genome() {}
Genome::Genome(const std::map<std::string, std::map<std::string, std::string>> genes) {
    std::map<std::string, std::map<std::string, std::string>>::const_iterator it;
    // make genes
    for (it = genes.begin(); it != genes.end(); ++it) {
        std::string name = it->first;
        std::map<std::string, std::string> expression = it->second;
        mGenes.push_back(Gene(name, expression));
    }
    std::cout << "Made Genome" << std::endl;
}

}  // end audiogen
