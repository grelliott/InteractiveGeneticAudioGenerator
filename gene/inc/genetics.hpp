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

#pragma once

#include <utility>
#include <memory>

#include "instruction.hpp"

namespace audiogene {

class Genetics {
    // forward-declare implementation class
    class Impl;
    std::unique_ptr<Impl> _impl;
    const Impl* Pimpl() const { return _impl.get(); }
    Impl* Pimpl() { return _impl.get(); }
 public:
    explicit Genetics(const double mutationProbability);
    ~Genetics();

    Genetics(Genetics&& rhs) = delete;
    Genetics& operator=(Genetics&& rhs) = delete;

    Genetics(const Genetics& rhs) = delete;
    Genetics& operator=(const Genetics& rhs) = delete;

    static Instructions create(const Instructions& seed);
    Instructions combine(const std::pair<Instructions, Instructions>& parents) const noexcept;
    Instructions mutate(const Instructions& instructions) const noexcept;
};

}  // namespace audiogene
