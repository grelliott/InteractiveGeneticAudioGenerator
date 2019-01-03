#pragma once

#include <vector>
#include <string>

#include "gene.hpp"

namespace audiogen {

    class Genome {
        std::vector<Gene> mGenes;
        public:
            Genome();
            Genome(const std::map<std::string, std::map<std::string, std::string>> genes);
            ~Genome() {};
    };
}  // end audiogen
