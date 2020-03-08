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

#include "instruction.hpp"

namespace audiogene {

Instruction::Instruction() {}

Instruction::Instruction(AttributeName name, Expression expression) :
    mName(name),
    mExpression(expression) {
}

Instruction::Instruction(AttributeName name, std::map<std::string, std::string> expression) :
		mName(name) {
    mExpression = {};
    mExpression.min = std::stoi(expression["min"]);
    mExpression.max = std::stoi(expression["max"]);
    mExpression.current = std::stoi(expression["current"]);
    mExpression.round = expression["round"] == "true";

    if (expression["activates"] == "OnBar") {
        mExpression.activates = ExpressionActivates::OnBar;
    } else if (expression["activates"] == "OverBar") {
        mExpression.activates = ExpressionActivates::OverBar;
    } else {
        mExpression.activates = ExpressionActivates::OnBar;
    }
}

AttributeName Instruction::name() const {
    return mName;
}

Expression Instruction::expression() const {
    return mExpression;
}

}  // namespace audiogene
