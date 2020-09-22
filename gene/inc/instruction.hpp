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

#include <cstdint>
#include <map>
#include <vector>
#include <string>

namespace audiogene {

enum class ExpressionActivates {
    OnBar,  //!< Make the change on the next bar
    OverBar  //!< Gradually make the change over the next bar
};

struct Expression {
    double min;
    double max;
    double current;
    bool round;
    ExpressionActivates activates;

    explicit Expression(const std::map<std::string, std::string>& d) {
        try {
            min = std::stoi(d.at("min"));
            max = std::stoi(d.at("max"));
            current = std::stoi(d.at("current"));
            round = d.at("round") == "true";

            if (d.at("activates") == "OnBar") {
                activates = ExpressionActivates::OnBar;
            } else if (d.at("activates") == "OverBar") {
                activates = ExpressionActivates::OverBar;
            } else {
                activates = ExpressionActivates::OnBar;
            }
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Failed to create expression");
        }
    }

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Expression &obj) {
        return os << "current: " << obj.current << ", min: " << obj.min << ", max: " << obj.max;
    }
};

using AttributeName = std::string;

class Instruction {
    const AttributeName _name;
    const Expression _expression;
 public:
    explicit Instruction(AttributeName name, const Expression& expression);

    auto name() const -> AttributeName;
    auto expression() const -> Expression;

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Instruction &obj) {
        return os << "Instruction " << obj.name() <<  ": " << obj.expression();
    }
};

using Instructions = std::map<AttributeName, Instruction>;

}  // namespace audiogene

