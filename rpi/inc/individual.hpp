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

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

#include "instruction.hpp"

namespace audiogene {

class Individual {
    std::shared_ptr<spdlog::logger> _logger;
    Instructions _instructions;
    uint32_t _id;

    static uint32_t s_id;
 public:
    Individual() {}
    /*! Create an individual from config values */
    explicit Individual(const std::map<std::string, std::map<std::string, std::string>> instructions);
    explicit Individual(const Instructions instructions);
    ~Individual() = default;

    Instructions instructions() const noexcept;
    Instruction instruction(const std::string& name) const noexcept;

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Individual &obj) {
        os << "Individual " << obj._id << std::endl;
        for (const std::pair<AttributeName, Instruction> &i : obj._instructions) {
            os << "\t" << i.second << "\n";
        }
        return os;
    }
};

}  // namespace audiogene
