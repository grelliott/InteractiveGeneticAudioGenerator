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
#include <random>
#include <ctime>
#include <cmath>

namespace audiogen {

Population::Population():
    mSize(0),
    mGeneration(0) {}

Population::Population(const uint8_t n, const Individual& seed):
    mSize(n),
    mGeneration(0) {
    _logger = spdlog::get("log");
    _logger->info("Making {} individuals from {}", n, seed);
    mRng.seed(std::chrono::system_clock::now().time_since_epoch().count());

    // AttributeSorter sorter;
    // go through seed and add to criteria
    Criteria criteria = initializeCriteria(seed);
    mSorter.setCriteria(criteria);

    mIndividuals = std::set<Individual, AttributeSorter>(mSorter);
    initializePopulation(seed);

    // For cross-breeding, randomly pick each of the attribute's values between two parents
    // For mutation, determine some threshold for mutations to occur,
    // and some deviation from the current value that a mutation could result it
    // For criteria, they're a k/v map of attributes and current inputs
    // rate-limit how often input buttons can have an effect on the criteria,
    // and how strong an effect it has on moving the criteria
}

Population::~Population() {}

Criteria Population::initializeCriteria(const Individual& seed) {
    Criteria criteria;
    for (Individual::const_iterator it = seed.cbegin(); it != seed.cend(); ++it) {
        std::string name(it->name());
        Expression expression(it->expression());
        Criterion criterion {};
        criterion.min = expression.min;
        criterion.max = expression.max;
        criterion.current = expression.current;
        criteria[name] = criterion;
    }
    return criteria;
}


void Population::updateCriterion(const AttributeName name) {
    _logger->info("+updateCriterion: {}", name);
    mSorter.updateCriterion(name, true);
}

void Population::initializePopulation(const Individual& seed) {
    // we need to make mSize copies of seed and put them into the collection of Individuals
    for (int i = 0; i < mSize; i++) {
        Attributes newAttributes = {};
        // Go through each Attribute in the seed and determine a new value based on a random standard deviation
        for (Individual::const_iterator it = seed.cbegin(); it != seed.cend(); ++it) {
            std::string name(it->name());
            Expression expression(it->expression());
            std::normal_distribution<> d(expression.current, 5);
            double v;
            do {
                v = d(mRng);
            } while ( v < expression.min || v > expression.max );
            if (expression.round) {
                v = std::round(v);
            }
            expression.current = v;
            newAttributes.emplace_back(Attribute(name, expression, mGeneration));
        }
        mIndividuals.insert(Individual(newAttributes));
    }
}

std::pair<Individual, Individual> Population::getParents(Individuals fittest) {
    std::uniform_int_distribution<> choose(0, fittest.size() - 1);

    int parent1Index = choose(mRng);
    Individuals::iterator it = fittest.begin();
    std::advance(it, parent1Index);
    const Individual parent1 = *it;
    // reset iterator to start
    it = fittest.begin();
    // Don't use the same parent twice
    int parent2Index;
    do {
        parent2Index = choose(mRng);
    } while (parent2Index == parent1Index);
    std::advance(it, parent2Index);
    const Individual parent2 = *it;
    return std::make_pair(parent1, parent2);
}

Individual Population::breed(std::pair<Individual, Individual> parents) {
    Attributes attributes;

    // Go through each attribute, flip a coin, and pick one or the other and give one to the child
    // Or determine some new value based on the two of the parents
    // For now, go basic and just pick one

    for (Individual::const_iterator it = parents.first.cbegin(); it != parents.first.cend(); ++it) {
        const Attribute parent1 = *it;
        const Attribute parent2 = parents.second.getAttribute(parent1.name());
        std::string attributeName = parent1.name();
        Expression expression = parent1.expression();
        std::array<decltype(expression.current), 4> intervals {
            static_cast<decltype(expression.current)>(expression.min),
            std::min(parent1.expression().current, parent2.expression().current),
            std::max(parent1.expression().current, parent2.expression().current),
            static_cast<decltype(expression.current)>(expression.max)
        };
        decltype(expression.current) desired = mSorter.get(attributeName);
        _logger->info("Desired current for {} is {}", attributeName, desired);
        decltype(expression.current) middle = 80.0;
        // normalize desired within range
        decltype(middle) weightingLR = (desired - expression.min) / (expression.max - expression.min);
        decltype(middle) weightLeft = (100.0 - middle) * (1 - weightingLR);
        decltype(middle) weightRight = (100.0 - middle) * weightingLR;
        _logger->info("Weightings are: left {}, middle {}, right {}", weightLeft, middle, weightRight);
        std::array<decltype(middle), 3> weights {
            weightLeft,
            middle,
            weightRight
        };

        // This performs a mutation on the new value
        // std::normal_distribution<> distribution((parent1.expression().current + parent2.expression().current)/2,
        //         std::abs(parent1.expression().current - parent2.expression().current)*2);
        // std::piecewise_constant_distribution<decltype(expression.current)> distribution(intervals.begin(),
        //         intervals.end(), weights.begin());
        std::piecewise_linear_distribution<decltype(expression.current)> distribution(intervals.begin(),
                intervals.end(), weights.begin());

        expression.current = distribution(mRng);
        if (expression.round) {
            expression.current = std::round(expression.current);
        }
        Attribute newAttribute(attributeName, expression, mGeneration);
        attributes.emplace_back(newAttribute);
    }
    return Individual(attributes);
}

void Population::nextGeneration(const uint8_t n) {
    mGeneration = mGeneration + 1;
    // copy fittest to temporary
    Individuals fittest;
    Individuals::iterator it = mIndividuals.begin();
    std::advance(it, n-1);
    fittest.insert(mIndividuals.begin(), it);

    // Remove unfittest
    it = mIndividuals.begin();
    std::advance(it, n);
    for (; it != mIndividuals.end(); ) {
        mIndividuals.erase(it++);
    }

    uint8_t maxTries = 8;
    uint8_t tries = 0;
    // add children back in until we have enough
    while (mIndividuals.size() < mSize && tries++ < maxTries) {
        std::pair<Individual, Individual> parents = getParents(fittest);
        // cross-breed top (they're also getting mutated)
        Individual newChild = breed(parents);
        mIndividuals.insert(newChild);
    }
}

Individual Population::fittest() {
    return *mIndividuals.begin();
}

}  // namespace audiogen
