/*
 * Copyright 2020 Grant Elliott <grant@grantelliott.ca>
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

#include "performance.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "audience.hpp"
#include "individual.hpp"
#include "midi.hpp"
#include "musician.hpp"
#include "osc.hpp"
#include "population.hpp"
#include "spi.hpp"

namespace audiogene {

Performance::Performance(const YAML::Node& config):
        _logger(spdlog::get("log")),
        _config(config) {
    seatAudience();
    assembleMusicians();
}

void Performance::seatAudience() {
    audiogene::Audience* audienceSource;

    YAML::Node inputNode;
    try {
        inputNode = _config["input"];
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("No input source configured");
    }

    std::string inputType;
    try {
        inputType = inputNode["type"].as<std::string>();
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Input source misconfigured");
    }

    if (inputType == "midi") {
        _logger->info("Input type is MIDI");
        const std::string inputName = (inputNode["name"] != nullptr) ? inputNode["name"].as<std::string>() : "";
        const KeyMap mapping = (inputNode["map"] != nullptr) ? inputNode["map"].as<KeyMap>() : KeyMap();
        audienceSource = new audiogene::MIDI(inputName, mapping);
    } else if (inputType == "spi") {
        _logger->info("Input type is SPI");
        audienceSource = new audiogene::SPI();
    } else {
        throw std::runtime_error("Unknown input type " + inputType);
    }

    // Set our audience to the source
    audience.reset(audienceSource);

    if (!audience->prepare()) {
        _logger->error("Failed to prepare input!");
        throw std::runtime_error("Failed to prepare input");
    }
    _logger->info("Input prepared");
}

void Performance::assembleMusicians() {
    std::string scAddr;
    std::string scPort;
    try {
        YAML::Node scNode(_config["SuperCollider"]);
        scAddr = scNode["addr"].as<std::string>();
        scPort = scNode["port"].as<std::string>();
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Missing SuperCollider config");
    }

    std::string oscPort;
    try {
        YAML::Node oscNode = _config["OSC"];
        oscPort = oscNode["port"].as<std::string>();
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Missing OSC config");
    }

    musician = std::make_unique<OSC>(oscPort, scAddr, scPort);
    // musician->send("/notify", "1");
}

auto Performance::play() -> std::future<void> {
    return std::async(std::launch::async, [this] () {
        // The input is from an audience
        // So we want an audience to guide the presentation
        // An audience gives feedback on various criteria
        // The population takes that feedback and determines which of its individuals best represent that feedback
        // and the next attempt tries to meet these expectations
        auto preferencesQueue = std::make_shared<moodycamel::BlockingConcurrentQueue<Preferences>>();
        audience->writeToPreferences(preferencesQueue);

        // Initialize preferences of audience
        KeyMap attributes;
        try {
            attributes = _config["genes"].as<KeyMap>();
        } catch (const YAML::Exception& e) {
            throw std::runtime_error("Missing audience attributes");
        }
        audience->initializePreferences(attributes);

        // Generate potential Conductors
        double mutationProbability;
        size_t populationSize;
        size_t topN;
        try {
            mutationProbability = _config["mutationProb"].as<double>();
            populationSize = _config["populationSize"].as<int>();
            topN = _config["keepFittest"].as<int>();
        } catch (const YAML::Exception& e) {
            throw std::runtime_error("Genetics misconfigured");
        }
        Individual seed(attributes);
        Population conductors(populationSize, seed, mutationProbability, topN);
        _logger->info("Initial population: {}", conductors);

        // Connect audience to conductor population
        // The conductors should keep asking for the reaction of the audience
        conductors.setPreferences(preferencesQueue);

        _logger->flush();

        // Make new generations
        uint8_t i = 0;
        while (musician->requestConductor()) {
            // Get the latest preferences from the audience
            audience->gatherPreferences();
            std::cout << "loop " << +i++ << std::endl;
            _logger->info("Getting new generation");
            conductors.nextGeneration();
            _logger->info("New population: {}", conductors);
            musician->setConductor(conductors.fittest());
            _logger->flush();
        }
    });
}

}  // namespace audiogene
