/*
 * Copyright 2018 Grant Elliott <grant@grantelliott.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <gflags/gflags.h>
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <map>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <csignal>

#include "osc.hpp"
#include "spi.hpp"
#include "individual.hpp"
#include "musician.hpp"
#include "audience.hpp"
#include "population.hpp"

// Command line argument flags
DEFINE_string(config, "", "Configuration for the genetics");
DEFINE_string(log, "out.log", "Logfile path");

volatile sig_atomic_t stop = 0;
void breakHandler(int s) {
    (void)s;
    stop = 1;
}

int main(int argc, char* argv[]) {
    // properly clean up on ctrl+c
    signal(SIGINT, breakHandler);

    // read arguments
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::shared_ptr<spdlog::logger> logger;
    try {
        // Initialize logging
        logger = spdlog::basic_logger_st("log", FLAGS_log, true);
        logger->info("Logging initialized");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return -1;
    }

    // get config
    if (FLAGS_config.empty()) {
        std::cout << "Missing config" << std::endl;
        return 1;
    }
    logger->info("Loading config file {}", FLAGS_config);

    // load config
    YAML::Node config = YAML::LoadFile(FLAGS_config);
    logger->info("Config loaded: {}", config["name"]);


   	//TODO don't fix output to a specific protocol
    // We're trying to "conduct" an audio performance
    // by telling it how to play
    // The conductor is the most suitable individual from the current generation
    // it will them tell the audio player the criteria to use for playing

    // Initialize output (OSC -> SuperCollider)
    if (!config["SuperCollider"]) {
        std::cout << "Missing SuperCollider config" << std::endl;
        return -1;
    }
    YAML::Node scNode = config["SuperCollider"];
    std::cout << "Initializing OSC" << std::endl;
    std::unique_ptr<audiogene::Musician> musician(new audiogene::OSC(scNode["addr"].as<std::string>(), scNode["port"].as<std::string>()));

    //TODO test result
    musician->prepare();
	std::cout << "SC is ready" << std::endl;

    //TODO don't fix input to a given protocol
    // Consider consuming MIDI and mapping that to each attribute
    // use an interface an instance of an input needs to honour

    // Initialize input
    //Audience audience = Midi();
	std::shared_ptr<audiogene::Audience> audience(new audiogene::SPI());
	//TODO test result
	audience->prepare();


    // Initialize genetics
    if (!config["genes"]) {
        logger->error("Missing required config item {}", "genes");
        return -1;
    }
    YAML::Node genesNode = config["genes"];

    std::vector<std::string> geneKeys;
    for (auto geneNode : genesNode) {
        geneKeys.emplace_back(geneNode.first.as<std::string>());
    }

    std::map<std::string, std::map<std::string, std::string>> attributes =
        genesNode.as<std::map<std::string, std::map<std::string, std::string>>>();

    // The input is from an audience
    // So we want an audience to guide the presentation
    // An audience gives feedback on various criteria
    // The population takes that feedback and determines which of its individuals best represent that feedback
    // and the next attempt tries to meet these expectations
    // Connect audience to population
    //pop.setAudience(audience);

    // Initialize the founder generation
    audiogene::Individual seed(attributes);
    audiogene::Population pop(config["populationSize"].as<int>(), seed, audience);
    logger->info("Initial population: {}", pop);


	// pop.onPreferencesUpdated(audience.preferencesUpdated());

    //TODO Use streams to link Population and OSC

    // Attach population to output
    audiogene::Individual conductor = pop.fittest();
    musician->receiveInstructions(conductor.instructions());


    // Run the thing
    uint8_t loops = 16;
    uint8_t topN = config["keepFittest"].as<int>();
    uint8_t i = 0;
    // Make new generations
    while (i < loops && stop == 0) {
        std::cout << "loop " << +i << std::endl;
        logger->info("Getting new generation from top {} individuals", topN);
        pop.nextGeneration(topN);
        logger->info("New population: {}", pop);
        //TODO Get audience preferences here...?
        musician->setConductor(pop.fittest(/* audience.preferences() */));
        sleep(8);
        i++;
    }

    stop = 1;
    //spiListener.join();
    return 0;
}
