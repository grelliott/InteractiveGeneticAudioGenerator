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
#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace audiogen {
enum class ExpressionActivates {
    OnBar,
    OverBar
};
struct Expression {
    uint32_t min;
    uint32_t max;
    double current;
    bool round;
    ExpressionActivates activates;

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Expression &obj) {
        return os << "current: " << obj.current << ", min: " << obj.min << ", max: " << obj.max;
    }
};
typedef std::string AttributeName;

class Attribute {
    AttributeName mName;
    Expression mExpression;
    uint8_t mGeneration;
 public:
    Attribute();
    Attribute(AttributeName name, std::map<std::string, std::string> expression, uint8_t generation);
    Attribute(AttributeName name, Expression expression, uint8_t generation);
    ~Attribute() {}
    AttributeName name() const;
    Expression expression() const;

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Attribute &obj) {
        return os << "Attribute " << obj.mName << ", generation  " << +obj.mGeneration << ": " << obj.mExpression;
    }
};

}  // namespace audiogen

