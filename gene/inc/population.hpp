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

#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "audience.hpp"
#include "blockingqueue.hpp"
#include "genetics.hpp"
#include "individual.hpp"
#include "math.hpp"

namespace audiogene {

constexpr uint8_t PREFERENCES_WAIT_FOR_S = 5;

using Individuals = std::vector<Individual>;

class Population {
    mutable std::shared_ptr<spdlog::logger> _logger;
    const Math _math;
    const Genetics _genetics;

    const size_t _size;
    Individuals _individuals;
    uint32_t _generation;
    const size_t _topN;

    // When it's time to create a new generation, get preferences from the audience
    // and sort individuals based on that
    Preferences _audiencePreferences;
    std::timed_mutex _havePreferences;

    void initializePopulation(const Individual& seed);
    void sortPopulation();
    auto similarity(const Individual& individual) -> double;

    // These are related to the genetics of a population
    // Maybe these should be in a different class
    auto getParents(const Individuals& fittest) -> std::pair<Individual, Individual>;
    auto breed(const std::pair<Individual, Individual>& parents) -> Individual;
    void mutate(Individual child);

 public:
    Population() = delete;
    Population(const uint8_t n, const Individual& seed, const double mutationProbability, const size_t topN);
    ~Population() = default;

    void setPreferences(const std::shared_ptr<moodycamel::BlockingConcurrentQueue<Preferences>>& preferencesQueue);

    auto fittest() -> Individual;

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
