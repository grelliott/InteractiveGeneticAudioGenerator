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

#include "individual.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <cassert>
#include <map>
#include <string>

namespace audiogene {

uint32_t Individual::s_id = 0;

Individual::Individual(const std::map<std::string, std::map<std::string, std::string>>& instructions):
        _id(s_id++) {
    std::remove_reference<decltype(instructions)>::type::const_iterator it;
    for (it = instructions.begin(); it != instructions.end(); ++it) {
        const std::string name = it->first;
        const std::map<std::string, std::string> expression = it->second;
        _instructions.emplace(name, Instruction(name, expression));
    }
}

Individual::Individual(const Instructions& instructions):
        _id(s_id++),
        _instructions(instructions) {
    // empty constructor
}

Instruction Individual::instruction(const std::string& name) const {
    try {
        return _instructions.at(name);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Failed to find instruction " + name);
    }
}

Instructions Individual::instructions() const noexcept {
    // perhaps stream something?
    // Either way, give instructions to musician
    return _instructions;
}

}  // namespace audiogene
