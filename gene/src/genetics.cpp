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

#include <cmath>
#include <memory>

#include "math.hpp"

namespace audiogene {

// Keep implementation header in source so it's not included
class Genetics::Impl {
    mutable std::shared_ptr<spdlog::logger> _logger;
    const Math _math;
    const double _mutationProbability;

    Expression mutateExpression(const Expression& orig,
                                const std::function<double(const Expression&)>&& distribution) const noexcept;

 public:
    explicit Impl(const double mutationProbability);
    ~Impl() = default;

    Instructions create(const Instructions& seed) const noexcept;
    Instructions combine(const std::pair<Instructions, Instructions>& parents) const noexcept;
    Instructions mutate(const Instructions& instructions) const noexcept;
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

Instructions Genetics::create(const Instructions& seed) const noexcept {
    return Pimpl()->create(seed);
}

Instructions Genetics::combine(const std::pair<Instructions, Instructions>& parents) const noexcept {
    return Pimpl()->combine(parents);
}

Instructions Genetics::mutate(const Instructions& instructions) const noexcept {
    return Pimpl()->mutate(instructions);
}


//
// Implementation
//
Genetics::Impl::Impl(const double mutationProbability):
        _logger(spdlog::get("log")),
        _mutationProbability(mutationProbability) {
    // Empty constructor
}

Instructions Genetics::Impl::create(const Instructions& seed) const noexcept {
    Instructions newInstructions;
    for (const std::pair<AttributeName, Instruction>& i : seed) {
        const AttributeName name(i.first);
        Expression newExpression(i.second.expression());
        if (newExpression.round) {
            newExpression.current = std::round(newExpression.current);
        }
        newInstructions.emplace(name, Instruction(name, newExpression));
    }
    return newInstructions;
}

Instructions Genetics::Impl::combine(const std::pair<Instructions, Instructions>& parents) const noexcept {
    const Instructions parent1 = parents.first;
    const Instructions parent2 = parents.second;
    Instructions childInstructions;

    for (const auto& kv : parent1) {
        const AttributeName attributeName = kv.first;
        if (_math.flipCoin()) {
            childInstructions.emplace(attributeName, parent1.at(attributeName));
        } else {
            childInstructions.emplace(attributeName, parent2.at(attributeName));
        }
    }
    return childInstructions;
}

Instructions Genetics::Impl::mutate(const Instructions& instructions) const noexcept {
    Instructions newInstructions;

    for (const auto& kv : instructions) {
        const AttributeName attributeName = kv.first;
        const Instruction instruction = kv.second;

        // Check if we should mutate or not
        if (_math.didEventOccur(_mutationProbability)) {
            // do the mutation thing
            Expression mutatedExpression(mutateExpression(instruction.expression(), [this] (const Expression& expression) {
                // This is the mutation function
                // It can be swapped with other mutation functions
                return _math.normalDistribution(expression.current, _math.stddev(expression.min, expression.max));
            }));

            newInstructions.emplace(attributeName, Instruction(attributeName, mutatedExpression));
        } else {
            newInstructions.emplace(attributeName, instruction);
        }
    }

    return newInstructions;
}

Expression Genetics::Impl::mutateExpression(const Expression& orig,
                                            const std::function<double(const Expression&)>&& distribution) const noexcept {
    Expression mutatedExpression(orig);
    do {
        mutatedExpression.current = distribution(orig);
    } while (!_math.inRange(mutatedExpression.current, mutatedExpression.min, mutatedExpression.max));

    if (mutatedExpression.round) {
        mutatedExpression.current = std::round(mutatedExpression.current);
    }
    return mutatedExpression;
}

}  // namespace audiogene
