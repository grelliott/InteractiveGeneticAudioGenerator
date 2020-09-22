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

#include <spdlog/spdlog.h>
#include <rtmidi/RtMidi.h>

#include <string>
#include <map>
#include <utility>
#include <memory>

#include "audience.hpp"

namespace audiogene {

constexpr unsigned char NOTE_OFF = 0b10000000;
constexpr unsigned char NOTE_ON = 0b10010000;

class MIDI: public Audience {
    std::shared_ptr<spdlog::logger> _logger;
    const std::string& _name;
    // key => {attribute, direction}
    const std::map<int, std::pair<AttributeName, int>> _mapping;
    std::unique_ptr<RtMidiIn> midiin;

 public:
    MIDI();
    // mapping is {"attribute": {"direction":"key"},...}
    MIDI(const std::string& name, const std::map<AttributeName, std::map<std::string, std::string>>& mapping);
    ~MIDI() final = default;

    auto prepare() -> bool final;
};

}  // namespace audiogene
