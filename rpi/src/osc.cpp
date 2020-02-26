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

#include <spdlog/spdlog.h>
#include <lo/lo.h>

namespace audiogen {

bool OSC::msIsSCReady = false;
lo_server_thread OSC::msSt = nullptr;

OSC::OSC():
    OSC(nullptr, "57120") {}

OSC::OSC(const std::string& serverIp, const std::string& serverPort):
    serverAddr(lo_address_new(serverIp.c_str(), serverPort.c_str())) {
        _logger = spdlog::get("log");
        _logger->info("OSC Initialized. {}:{}", serverIp, serverPort);
}

OSC::~OSC() {}

void OSC::prepare() {
	//TODO can we listen for something instead of polling outself?
	// wait for SuperCollider by querying /notify
    // need to set up an OSC server for this
    while (!isSCReady()) {
        sleep(1);  // wait 1 second
    }
}

bool OSC::isSCReady() {
    if (msIsSCReady) {
        if (msSt) {
            lo_server_thread_free(msSt);
            lo_send(serverAddr, "/notify", "i", 0);
        }
        return true;
    }
    if (msSt) {
        return false;
    }

    msSt = lo_server_thread_new(nullptr, [] (int num, const char* m, const char* path) {
        // error handler
        std::cout << "OSC Server Error: num = " << num << " m = " << m << ", path = " << path << std::endl;
    });

    lo_server_thread_add_method(msSt, "/done", "s", [] (const char* path,
                const char* types,
                lo_arg** argv, int argc,
                void* data, void* user_data) -> int {
        (void)path;
        (void)types;
        (void)argv;
        (void)argc;
        (void)data;
        (void)user_data;
        msIsSCReady = true;
        return 1;
    }, nullptr);

    lo_server_thread_start(msSt);

    lo_send(serverAddr, "/notify", "i", 1);

    return false;
}

void OSC::receiveInstructions(const Instructions& instructions) {
	// instructions should contain a bunch of attributes and values
}

void OSC::setConductor(const Individual& conductor) {
    _logger->info("Setting new conductor {}", conductor);
    for (Individual::const_iterator it = conductor.cbegin(); it != conductor.cend(); ++it) {
        lo_send(serverAddr, std::string("/gene/"+it->name()).c_str(), "f", it->expression().current);
    }
}

bool OSC::send(const std::string& path, const std::string& msg) {
    lo_send(serverAddr, path.c_str(), "s", msg.c_str());
    return true;
}

}  // namespace audiogen
