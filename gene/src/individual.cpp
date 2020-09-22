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

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace audiogene {

uint32_t Individual::s_id = 0;

auto convertMapToInstructions(const std::map<std::string, std::map<std::string, std::string>>& instructions) -> Instructions {
    Instructions r;
    std::remove_reference<decltype(instructions)>::type::const_iterator it;
    for (it = instructions.begin(); it != instructions.end(); ++it) {
        r.emplace(it->first, Instruction(it->first, Expression(it->second)));
    }
    return r;
}

Individual::Individual(const std::map<std::string, std::map<std::string, std::string>>& instructions):
        _id(s_id++),
        _instructions(convertMapToInstructions(instructions)) {
    // empty constructor
}

Individual::Individual(Instructions instructions):
        _id(s_id++),
        _instructions(std::move(instructions)) {
    // empty constructor
}

auto Individual::instruction(const std::string& name) const -> Instruction {
    try {
        return _instructions.at(name);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Failed to find instruction " + name);
    }
}

auto Individual::instructions() const noexcept -> Instructions {
    return _instructions;
}

}  // namespace audiogene
