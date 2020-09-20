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

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <vector>
#include <utility>
#include <algorithm>
#include <random>
#include <memory>
#include <mutex>

#include "individual.hpp"
#include "genetics.hpp"
#include "audience.hpp"
#include "blockingqueue.hpp"

namespace audiogene {

using Individuals = std::vector<Individual>;

class Population {
    std::shared_ptr<spdlog::logger> _logger;
    //TODO finish moving to math
    mutable std::default_random_engine mRng;
    const Genetics _genetics;

    const size_t _size;
    Individuals _individuals;
    uint32_t _generation;
    const size_t _topN;

    // When it's time to create a new generation, get preferences from the audience
    // and sort individuals based on that
    mutable Preferences _audiencePreferences;
    std::timed_mutex _havePreferences;

    void initializePopulation(const Individual& seed);
    void sortPopulation();
    double similarity(const Individual& indvidual);

    // These are related to the genetics of a population
    // Maybe these should be in a different class
    std::pair<Individual, Individual> getParents(Individuals fittest);
    Individual breed(std::pair<Individual, Individual> parents);
    void mutate(Individual child);

 public:
    Population() = delete;
    Population(const uint8_t n, const Individual& seed, const double mutationProbability, const size_t topN);
    ~Population() = default;

    void setPreferences(std::shared_ptr<moodycamel::BlockingConcurrentQueue<Preferences>> preferencesQueue);

    const Individual fittest();

    void nextGeneration();

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Population &obj) {
        os << "Population \n";
        for (const Individual &individual : obj._individuals) {
            os << "\t" << individual << std::endl;
        }
        return os;
    }
};

}  // namespace audiogene
