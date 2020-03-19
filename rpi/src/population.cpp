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

namespace audiogene {

Population::Population(const uint8_t n, const Individual& seed, const std::shared_ptr<Audience> audience):
	    _logger(spdlog::get("log")),
	    mSize(n),
	    mGeneration(0),
	    mAudience(audience) {
	_logger->info("Initializing RNG");
    mRng.seed(std::chrono::system_clock::now().time_since_epoch().count());

	_logger->info("Making {} individuals from {}", n, seed);
    initializePopulation(seed);

    // For cross-breeding, randomly pick each of the attribute's values between two parents
    // For mutation, determine some threshold for mutations to occur,
    // and some deviation from the current value that a mutation could result it
    // For criteria, they're a k/v map of attributes and current inputs
    // rate-limit how often input buttons can have an effect on the criteria,
    // and how strong an effect it has on moving the criteria
}

void Population::initializePopulation(const Individual& seed) {
    // we need to make mSize copies of seed and put them into the collection of Individuals
    for (int i = 0; i < mSize; i++) {
        Instructions newInstructions = {};
        // Go through each Preference in the seed and determine a new value based on a random standard deviation
        //TODO change this from iterating on the seed to the seed's instructions
        for (Instructions::const_iterator it = seed.instructions().cbegin(); it != seed.instructions().cend(); ++it) {
        	// The seed's instruction
            std::string name(it->name());
            Expression expression(it->expression());

            // this can be pulled out into a new method
            // it's getting a new expression over a normal distribution
            //TODO get stddev from expression somehow...?
            double stddev = 1.0;
            std::normal_distribution<> d(expression.current, stddev);
            double v;
            // This is to keep sampling new values from the overall distribution until one is found within
            // the allowable range
            do {
                v = d(mRng);
            } while ( v < expression.min || v > expression.max );
            if (expression.round) {
                v = std::round(v);
            }
            expression.current = v;

            newInstructions.emplace_back(Instruction(name, expression));
        }
        mIndividuals.emplace_back(Individual(newInstructions));
    }
}

double Population::similarity(const Individual& indvidual) {
	double similarity = 0;
    for (Instructions::const_iterator it = indvidual.instructions().cbegin(); it != indvidual.instructions().cend(); ++it) {
        const double curVal = it->expression().current;
        const double ideal = mAudience->preferences()[it->name()].current;
        double sim = 1 - (std::abs(ideal - curVal)) / (it->expression().max - it->expression().min);
        similarity += sim;
    }
    return similarity;
}

void Population::sortPopulation() {
	std::sort(mIndividuals.begin(), mIndividuals.end(), [&] (const Individual& lhs, const Individual& rhs) -> bool {
        float lhsSimilarity = similarity(lhs);
        float rhsSimilarity = similarity(rhs);
        const uint8_t instructionCount = lhs.instructions().size();
        return (lhsSimilarity/instructionCount) > (rhsSimilarity/instructionCount);
    });
}

std::pair<Individual, Individual> Population::getParents(Individuals fittest) {
    std::uniform_int_distribution<> choose(0, fittest.size() - 1);

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
    Instructions instructions;

    // Go through each attribute, flip a coin, and pick one or the other and give one to the child
    // Or determine some new value based on the two of the parents
    // For now, go basic and just pick one

    for (Instructions::const_iterator it = parents.first.instructions().cbegin(); it != parents.first.instructions().cend(); ++it) {
        const Instruction parent1Instruction = *it;
        const Instruction parent2Instruction = parents.second.instruction(parent1Instruction.name());
        std::string instructionName = parent1Instruction.name();

        Expression expression = parent1Instruction.expression();
        std::array<decltype(expression.current), 4> intervals {
            static_cast<decltype(expression.current)>(expression.min),
            std::min(parent1Instruction.expression().current, parent2Instruction.expression().current),
            std::max(parent1Instruction.expression().current, parent2Instruction.expression().current),
            static_cast<decltype(expression.current)>(expression.max)
        };

        decltype(expression.current) desired = mAudience->preferences()[instructionName].current;
        _logger->info("Desired current for {} is {}", instructionName, desired);
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

        //TODO extract to mutate method
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
        Instruction newInstruction(instructionName, expression);
        instructions.emplace_back(newInstruction);
    }
    return Individual(instructions);
}

void Population::nextGeneration(const uint8_t n) {
    mGeneration = mGeneration + 1;
    // copy fittest to temporary
    Individuals fittest;
    Individuals::iterator it = mIndividuals.begin();
    std::advance(it, n-1);
    fittest.insert(fittest.end(), std::make_move_iterator(mIndividuals.begin()), std::make_move_iterator(it));

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
        mIndividuals.emplace_back(newChild);
    }
}

const Individual Population::fittest() {
    return mIndividuals.front();
}

}  // namespace audiogene
