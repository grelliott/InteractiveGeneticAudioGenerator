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

#include "population.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <thread>

namespace audiogene {

Population::Population(const uint8_t n,
                       const Individual& seed,
                       const double mutationProbability,
                       const size_t topN):
        _logger(spdlog::get("log")),
        _genetics(mutationProbability),
        _size(n),
        _generation(0),
        _topN(topN) {
    _logger->info("Initializing RNG");
    mRng.seed(std::chrono::system_clock::now().time_since_epoch().count());

    _logger->info("Making {} individuals from {}", n, seed);
    initializePopulation(seed);
}

void Population::initializePopulation(const Individual& seed) {
    std::generate_n(std::back_inserter(_individuals), _size, [this, &seed] () -> Individual {
        return Individual(_genetics.create(seed.instructions()));
    });
}

double Population::similarity(const Individual& individual) {
    double similarity = 0;
    for (const std::pair<AttributeName, Instruction>& i : individual.instructions()) {
        Instruction instruction = i.second;
        // TODO(grant) move this to the math lib
        const double curVal = instruction.expression().current;
        const double ideal = _audiencePreferences.at(instruction.name()).current;
        similarity += 1 - (std::abs(ideal - curVal))
                      / (instruction.expression().max - instruction.expression().min);
    }
    return similarity;
}

void Population::sortPopulation() {
    // wait here until we have new preferences
    // or we've reached a timeout
    // TOOD(grant) change timer to take updateable preference
    bool haveLock = _havePreferences.try_lock_for(std::chrono::seconds(5));
    std::sort(_individuals.begin(), _individuals.end(), [this] (const Individual& lhs, const Individual& rhs) -> bool {
        return (similarity(lhs) / lhs.instructions().size()) > (similarity(rhs) / rhs.instructions().size());
    });
    if (haveLock) {
        _havePreferences.unlock();
    }
}

std::pair<Individual, Individual> Population::getParents(Individuals fittest) {
    std::uniform_int_distribution<int> choose(0, fittest.size() - 1);

    int parent1Index = choose(mRng);
    Individuals::iterator it = fittest.begin();
    std::advance(it, parent1Index);
    const Individual parent1 = *it;
    // Don't use the same parent twice
    int parent2Index;
    do {
        parent2Index = choose(mRng);
    } while (parent2Index == parent1Index);
    it = fittest.begin();
    std::advance(it, parent2Index);
    const Individual parent2 = *it;
    return std::make_pair(parent1, parent2);
}

Individual Population::breed(std::pair<Individual, Individual> parents) {
    return Individual(_genetics.mutate(
        _genetics.combine(std::make_pair(parents.first.instructions(), parents.second.instructions()))));
}

void Population::nextGeneration() {
    _generation = _generation + 1;
    // copy fittest to temporary
    Individuals fittest(_individuals.begin(), _individuals.begin() + _topN);
    // Remove unfittest individuals
    _individuals.erase(_individuals.begin() + _topN, _individuals.end());

    // Refill with new children
    std::generate_n(std::back_inserter(_individuals), _size - _topN, [this, &fittest] () -> Individual {
        return breed(getParents(fittest));
    });

    sortPopulation();
}

void Population::setPreferences(std::shared_ptr<moodycamel::BlockingConcurrentQueue<Preferences>> preferencesQueue) {
    std::thread t([preferencesQueue, this] () {
        while (true) {
            _havePreferences.lock();
            preferencesQueue->wait_dequeue(_audiencePreferences);
            _havePreferences.unlock();
        }
    });
    t.detach();
}

const Individual Population::fittest() {
    return _individuals.front();
}

}  // namespace audiogene
