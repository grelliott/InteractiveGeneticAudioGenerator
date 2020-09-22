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
#include <memory>
#include <thread>

#include "math.hpp"

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
    _logger->info("Making {} individuals from {}", n, seed);
    initializePopulation(seed);
}

void Population::initializePopulation(const Individual& seed) {
    std::generate_n(std::back_inserter(_individuals), _size, [&seed] () -> Individual {
        return Individual(Genetics::create(seed.instructions()));
    });
}

auto Population::similarity(const Individual& individual) -> double {
    double similarity = 0;
    for (const auto& i : individual.instructions()) {
        const Instruction instruction = i.second;
        const Expression expression(instruction.expression());
        const double ideal = _audiencePreferences.at(instruction.name()).current;
        similarity += _math.similarity(ideal, expression.current, expression.min, expression.max);
    }
    return similarity;
}

void Population::sortPopulation() {
    std::sort(_individuals.begin(), _individuals.end(), [this] (const Individual& lhs, const Individual& rhs) -> bool {
        return (similarity(lhs) / lhs.instructions().size()) > (similarity(rhs) / rhs.instructions().size());
    });
}

auto Population::getParents(const Individuals& fittest) -> std::pair<Individual, Individual> {
    std::pair<int, int> parents = _math.uniquePair(fittest);
    return std::make_pair(fittest.at(parents.first), fittest.at(parents.second));
}

auto Population::breed(const std::pair<Individual, Individual>& parents) -> Individual {
    return Individual(_genetics.mutate(
        _genetics.combine(std::make_pair(parents.first.instructions(), parents.second.instructions()))));
}

void Population::nextGeneration() {
    // wait here until we have new preferences
    // or we've reached a timeout
    // TOOD(grant) change timer to take updateable preference
    bool haveLock = _havePreferences.try_lock_for(std::chrono::seconds(PREFERENCES_WAIT_FOR_S));
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
    if (haveLock) {
        _havePreferences.unlock();
    }
}

void Population::setPreferences(const std::shared_ptr<moodycamel::BlockingConcurrentQueue<Preferences>>& preferencesQueue) {
    std::thread t([preferencesQueue, this] () {
        while (true) {
            _havePreferences.lock();
            preferencesQueue->wait_dequeue(_audiencePreferences);
            _havePreferences.unlock();
        }
    });
    t.detach();
}

auto Population::fittest() -> Individual {
    return _individuals.front();
}

}  // namespace audiogene
