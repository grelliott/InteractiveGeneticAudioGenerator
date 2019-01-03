
#include <iostream>
#include <map>
#include <string>

#include <gflags/gflags.h>
#include <yaml-cpp/yaml.h>

#include "genome.hpp"

DEFINE_string(config, "", "Configuration for the genetics");

int main(int argc, char* argv[]) {
    // read arguments
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    // get config
    if (FLAGS_config.empty()) {
        std::cout << "Missing config" << std::endl;
        return 1;
    }

    // open file
    // parse
    YAML::Node config = YAML::LoadFile(FLAGS_config);
    if (!config["genes"]) {
        std::cout << "Missing genes in config" << std::endl;
    }
    YAML::Node genesNode = config["genes"];
    std::map<std::string, std::map<std::string, std::string>> genes = genesNode.as<std::map<std::string, std::map<std::string, std::string>>>();
    // Ingest genetics 
    audiogen::Genome genome = audiogen::Genome(genes);
    
    // initialize input

    return 0;
}
