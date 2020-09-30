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

#include "osc.hpp"

#include <lo/lo.h>
#include <lo/lo_cpp.h>
#include <spdlog/spdlog.h>

#include <condition_variable>
#include <future>
#include <mutex>

namespace audiogene {

OSC::OSC():
    OSC(&DEFAULT_CLIENT_PORT[0], &DEFAULT_SERVER_ADDR[0], &DEFAULT_SERVER_PORT[0]) {}

OSC::OSC(const std::string& clientPort, const std::string& serverIp, const std::string& serverPort):
        _logger(spdlog::get("log")),
        client(clientPort),
        scLangServer(serverIp, serverPort) {
    if (!client.is_valid()) {
        _logger->warn("Invalid OSC Server: client");
        throw std::runtime_error("Failed to initialize OSC");
    }

    std::promise<void> isReadyPromise;
    std::future<void> isReady(isReadyPromise.get_future());

    client.set_callbacks(
            [this, &isReadyPromise] () {
                _logger->info("OSC client started {}", client.url(), client.port());
                isReadyPromise.set_value();
            },
            [this] () {
                _logger->info("OSC client finished");
            });

    client.add_method("/request", "s", [this] (lo_arg **argv, int len) {
        _logger->info("Request for new conductor");
        std::unique_lock<std::mutex> l(_nextMutex);
        _nextCV.notify_all();
    });

    client.start();

    // Wait until we've started and the promise is set
    isReady.get();

    // Tell SuperCollider to start playing music
    int r = scLangServer.send("/connected");

    if (r == -1) {
        _logger->error("Failed to initialize OSC");
    } else {
        _logger->info("OSC Initialized");
    }
}

 auto OSC::requestConductor() -> bool {
    std::unique_lock<std::mutex> l(_nextMutex);
    // TODO(grant) make this timer modifyable
    if (_nextCV.wait_for(l, std::chrono::seconds(REQUEST_WAIT_FOR_S)) != std::cv_status::timeout) {
        _logger->info("Didn't time out waiting for signal!");
    } else {
        _logger->info("Timed out waiting for signal!");
    }
    return true;
}

void OSC::setConductor(const Individual& conductor) {
    _logger->info("Setting new conductor {}", conductor);
    const Instructions instructions = conductor.instructions();
    for (const auto& kv : instructions) {
        const AttributeName attributeName = kv.first;
        const Instruction instruction = kv.second;
        lo::Message m;
        m.add_double(instruction.expression().current);
        int r = scLangServer.send(std::string("/gene/"+attributeName).c_str(), m);
        if (r == -1) {
            _logger->warn("Failed to send OSC message {}", attributeName);
        }
    }
    _logger->info("New conductor set", conductor);
}

auto OSC::send(const std::string& path, const std::string& msg) -> bool {
    lo::Message m;
    m.add_string(msg);
    return scLangServer.send(path, m) != -1;
}

}  // namespace audiogene
