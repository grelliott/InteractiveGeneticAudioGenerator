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
#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "musician.hpp"
#include "instruction.hpp"
#include "individual.hpp"

namespace audiogene {

class OSC: public Musician {
    std::shared_ptr<spdlog::logger> _logger;
    lo::ServerThread client;
    lo::Address scLangServer;

    std::mutex _nextMutex;
    std::condition_variable _nextCV;

    bool send(const std::string& path, const std::string& msg);
 public:
    OSC();
    OSC(const std::string& clientPort, const std::string& serverIp, const std::string& serverPort);
    ~OSC() = default;

    virtual void requestConductor();
    virtual void setConductor(const Individual& conductor) final;
};

}  // namespace audiogene
