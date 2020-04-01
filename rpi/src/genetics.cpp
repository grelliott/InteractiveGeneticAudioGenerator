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

#include "genetics.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <ctime>
#include <cmath>

namespace audiogene {

// Keep implementation header in source so it's not included
class Genetics::Impl {
    std::shared_ptr<spdlog::logger> _logger;
    mutable std::default_random_engine mRng;
    const double _mutationProbability;

    double stddev(const double min, const double max) const noexcept;

    // template??
    double normalDistribution(const Expression& expression) const noexcept;

    // template??
    double normalDistribution(const double mean, const double stddev) const noexcept;

    Instructions combine_randomZipper(const std::pair<Instructions, Instructions>& parents) const noexcept;
    Expression mutateExpression(const Expression orig,
                                std::function<double(const Expression&)>&& distribution) const noexcept;

 public:
    explicit Impl(const double mutationProbability);
    ~Impl() = default;

    Instructions create(const Instructions seed) const noexcept;
    Instructions combine(const std::pair<Instructions, Instructions>& parents) const noexcept;
    Instructions mutate(const Instructions instructions) const noexcept;
};


Genetics::Genetics(const double mutationProbability): _impl(new Impl(mutationProbability)) {}
Genetics::~Genetics() = default;
Genetics::Genetics(Genetics&& rhs) noexcept = default;
Genetics& Genetics::operator=(Genetics&& rhs) noexcept = default;
Genetics::Genetics(const Genetics& rhs): _impl(new Impl(*rhs._impl)) {}
Genetics& Genetics::operator=(const Genetics& rhs) {
    if (this != &rhs) {
        _impl.reset(new Impl(*rhs._impl));
    }
    return *this;
}

Instructions Genetics::create(const Instructions seed) const noexcept {
    return Pimpl()->create(seed);
}

Instructions Genetics::combine(const std::pair<Instructions, Instructions>& parents) const noexcept {
    return Pimpl()->combine(parents);
}

Instructions Genetics::mutate(const Instructions instructions) const noexcept {
    return Pimpl()->mutate(instructions);
}


//
// Implementation
//
Genetics::Impl::Impl(const double mutationProbability):
        _logger(spdlog::get("log")),
        _mutationProbability(mutationProbability) {
    _logger->info("Initializing RNG");
    mRng.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

Instructions Genetics::Impl::create(const Instructions seed) const noexcept {
    Instructions newInstructions;
    for (const std::pair<AttributeName, Instruction>& i : seed) {
        AttributeName name(i.first);
        Expression e = i.second.expression();
        Expression newExpression(e);
        do {
            newExpression.current = normalDistribution(e.current, stddev(e.min, e.max));
        } while ( newExpression.current < newExpression.min || newExpression.current > newExpression.max );
        if (newExpression.round) {
            newExpression.current = std::round(newExpression.current);
        }
        newInstructions[name] = Instruction(name, newExpression);
    }
    return newInstructions;
}

Instructions Genetics::Impl::combine(const std::pair<Instructions, Instructions>& parents) const noexcept {
    return combine_randomZipper(parents);
}


Instructions Genetics::Impl::mutate(const Instructions instructions) const noexcept {
    std::uniform_real_distribution<double> d(0.0, 1.0);
    Instructions newInstructions;

    for (const auto& kv : instructions) {
        AttributeName attributeName = kv.first;
        Instruction instruction = kv.second;
        // Check if we should mutate or not
        if (d(mRng) >= _mutationProbability) {
            // don't mutate, just add new instruction as-is
            newInstructions.emplace(attributeName, instruction);
        }
        // do the mutation thing
        Expression mutatedExpression(mutateExpression(instruction.expression(), [this] (const Expression& expression) -> double {
            // This is the distribution used to select a new expression
            return normalDistribution(expression);
        }));

        Instruction mutatedInstruction(attributeName, mutatedExpression);
        newInstructions.emplace(attributeName, mutatedInstruction);
    }

    return newInstructions;
}

Instructions Genetics::Impl::combine_randomZipper(const std::pair<Instructions, Instructions>& parents) const noexcept {
    // 50/50 chance of parent 1 vs parent 2
    std::uniform_int_distribution<int> d(0, 1);

    Instructions parent1 = parents.first;
    Instructions parent2 = parents.second;
    Instructions childInstructions;

    for (const auto& kv : parent1) {
        AttributeName attributeName = kv.first;
        // If 0, pick first parent
        // If 1, pick second
        if (d(mRng) == 0) {
            childInstructions.emplace(attributeName, parent1.at(attributeName));
            continue;
        }
        // Choice was 1, pick second parent's instructions
        try {
            childInstructions.emplace(attributeName, parent2.at(attributeName));
        } catch (const std::out_of_range& e) {
            // oops
            _logger->warn("Missing attribute {} in parent2", attributeName);
            // Just add first parents instruction item
            childInstructions.emplace(attributeName, parent1.at(attributeName));
        }
    }
    return childInstructions;
}


Expression Genetics::Impl::mutateExpression(const Expression orig,
                                            std::function<double(const Expression&)>&& distribution) const noexcept {
    Expression mutatedExpression(orig);
    do {
        mutatedExpression.current = distribution(orig);
    } while (mutatedExpression.current < mutatedExpression.min || mutatedExpression.current > mutatedExpression.max);
    return mutatedExpression;
}

double Genetics::Impl::stddev(const double min, const double max) const noexcept {
    return (max - min)/6;
}

double Genetics::Impl::normalDistribution(const Expression& expression) const noexcept {
    return normalDistribution(expression.current, stddev(expression.min, expression.max));
}

double Genetics::Impl::normalDistribution(const double mean, const double stddev) const noexcept {
    std::normal_distribution<double> d(mean, stddev);
    return d(mRng);
}

}  // namespace audiogene
