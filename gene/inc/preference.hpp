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

#include <map>
#include <string>

namespace audiogene {

using Attribute = std::map<std::string, std::string>;

struct Preference {
    double min;
    double max;
    double current;

    explicit Preference(const Attribute& attribute) {
        try {
            current = std::stoi(attribute.at("current"));
            min = std::stoi(attribute.at("min"));
            max = std::stoi(attribute.at("max"));
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Invalid attribute");
        }
    }
};

using AttributeName = std::string;
using Preferences = std::map<AttributeName, Preference>;

}  // namespace audiogene
