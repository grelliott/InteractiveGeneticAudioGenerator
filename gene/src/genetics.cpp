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

    auto mutateExpression(const Expression& orig,
            const std::function<double(const Expression&)>&& distribution) const noexcept -> Expression;

 public:
    explicit Impl(double mutationProbability);
    Impl(const Impl&) = delete;
    Impl(Impl&&) = delete;
    auto operator=(const Impl&) -> Impl& = delete;
    auto operator=(Impl&&) -> Impl& = delete;
    ~Impl() = default;


    static auto create(const Instructions& seed) -> Instructions;
    auto combine(const std::pair<Instructions, Instructions>& parents) const noexcept -> Instructions;
    auto mutate(const Instructions& instructions) const noexcept -> Instructions;
};

Genetics::Genetics(const double mutationProbability): _impl(new Impl(mutationProbability)) {}
Genetics::~Genetics() = default;

auto Genetics::create(const Instructions& seed) -> Instructions {
    return Impl::create(seed);
}

auto Genetics::combine(const std::pair<Instructions, Instructions>& parents) const noexcept -> Instructions {
    return Pimpl()->combine(parents);
}
auto Genetics::mutate(const Instructions& instructions) const noexcept -> Instructions {
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

// TODO Create individuals using an open-ended normal distribution with the median at the middle of the min/max
auto Genetics::Impl::create(const Instructions& seed) -> Instructions {
    Instructions newInstructions;
    for (const auto& i : seed) {
        const AttributeName name(i.first);
        Expression newExpression(i.second.expression());
        if (newExpression.round) {
            newExpression.current = std::round(newExpression.current);
        }
        newInstructions.emplace(name, Instruction(name, newExpression));
    }
    return newInstructions;
}

auto Genetics::Impl::combine(const std::pair<Instructions, Instructions>& parents) const noexcept -> Instructions {
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

auto Genetics::Impl::mutate(const Instructions& instructions) const noexcept -> Instructions {
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

auto Genetics::Impl::mutateExpression(const Expression& orig,
        const std::function<double(const Expression&)>&& distribution) const noexcept -> Expression {
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
