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

#include <cstdint>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>
#include <cmath>

namespace audiogene {

Population::Population(const uint8_t n,
                       const Individual& seed,
                       const double mutationProbability,
                       const std::shared_ptr<Audience> audience):
        _logger(spdlog::get("log")),
        _genetics(mutationProbability),
        mSize(n),
        mIndividuals(mSize),
        mGeneration(0),
        mAudience(audience) {
    _logger->info("Initializing RNG");
    mRng.seed(std::chrono::system_clock::now().time_since_epoch().count());

    _logger->info("Making {} individuals from {}", n, seed);
    initializePopulation(seed);
}

void Population::initializePopulation(const Individual& seed) {
    std::generate(mIndividuals.begin(), mIndividuals.end(), [this, &seed] () {
        return Individual(_genetics.create(seed.instructions()));
    });
}

double Population::similarity(const Individual& individual) {
    double similarity = 0;
    for (const std::pair<AttributeName, Instruction>& i : individual.instructions()) {
        Instruction instruction = i.second;
        const double curVal = instruction.expression().current;
        const double ideal = mAudience->preferences()[instruction.name()].current;
        similarity += 1 - (std::abs(ideal - curVal))
                      / (instruction.expression().max - instruction.expression().min);
    }
    return similarity;
}

void Population::sortPopulation() {
    std::sort(mIndividuals.begin(), mIndividuals.end(), [this] (const Individual& lhs, const Individual& rhs) -> bool {
        return (similarity(lhs) / lhs.instructions().size()) > (similarity(rhs) / rhs.instructions().size());
    });
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

void Population::nextGeneration(const uint8_t topN) {
    mGeneration = mGeneration + 1;
    // copy fittest to temporary
    Individuals fittest(mIndividuals.begin(), mIndividuals.begin() + topN);
    // Remove unfittest individuals
    mIndividuals.erase(mIndividuals.begin() + topN, mIndividuals.end());

    // Refill with new children
    std::generate_n(std::back_inserter(mIndividuals), mSize - topN, [this, &fittest] () -> Individual {
        return breed(getParents(fittest));
    });

    sortPopulation();
}

const Individual Population::fittest() {
    return mIndividuals.front();
}

}  // namespace audiogene
