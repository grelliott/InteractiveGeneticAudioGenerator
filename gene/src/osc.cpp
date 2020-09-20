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
    OSC("57130", "localhost", "57110") {}

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

    client.add_method("/next", "s", [this] (lo_arg **argv, int len) {
        (void)len;
        _logger->info("Client Next: {}", argv[0]->s);
        std::unique_lock<std::mutex> l(_nextMutex);
        _nextCV.notify_all();
    });

    client.start();

    // Wait until we've started and the promise is set
    isReady.get();

    // Tell SuperCollider to start playing music
    lo_send(scLangServer, "/notify", "i", 1);

    _logger->info("OSC Initialized");
}

bool OSC::requestConductor() {
    std::unique_lock<std::mutex> l(_nextMutex);
    // TODO(grant) make this timer modifyable
    if (_nextCV.wait_for(l, std::chrono::seconds(8)) != std::cv_status::timeout) {
        _logger->info("Didn't time out waiting for signal!");
    } else {
        _logger->info("Timed out waiting for signal!");
    }
    return true;
}

void OSC::setConductor(const Individual& conductor) {
    _logger->info("Setting new conductor {}", conductor);
    Instructions instructions = conductor.instructions();
    for (const auto& kv : instructions) {
        AttributeName attributeName = kv.first;
        Instruction instruction = kv.second;
        // TODO(grant) Determine the type of data being sent
        lo_send(scLangServer, std::string("/gene/"+attributeName).c_str(), "f", instruction.expression().current);
    }
    _logger->info("New conductor set", conductor);
}

bool OSC::send(const std::string& path, const std::string& msg) {
    lo_send(scLangServer, path.c_str(), "s", msg.c_str());
    return true;
}

}  // namespace audiogene
