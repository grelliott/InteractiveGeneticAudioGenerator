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
#include <lo/lo_cpp.h>

#include <future>

namespace audiogene {

OSC::OSC():
    OSC(nullptr, "57120") {}

OSC::OSC(const std::string& serverIp, const std::string& serverPort):
		_logger(spdlog::get("log")),
	    serverAddr(lo_address_new(serverIp.c_str(), serverPort.c_str())),
	    st(9000) {
	if (!st.is_valid()) {
		_logger->warn("Invalid OSC Server");
    	return;
    }

	std::promise<void> isReadyPromise;
	std::future<void> isReady(isReadyPromise.get_future());

    st.set_callbacks([this, &isReadyPromise] () {
    	_logger->info("OSC Thread started");
    	isReadyPromise.set_value();
    },
    				 [this] () {
		_logger->info("OSC Thread finished");
	});

	st.add_method("done", "s", [this] (lo_arg **argv, int len) {
		(void)len;
		_logger->info("Done: %s", argv[0]->s);
	});

	st.start();

	// Notify SuperCollider we're ready
    lo_send(serverAddr, "/notify", "i", 1);

    // Wait until we've started and the promise is set
    isReady.get();
    _logger->info("OSC Initialized. {}:{}", serverIp, serverPort);
}

void OSC::setConductor(const Individual& conductor) {
    _logger->info("Setting new conductor {}", conductor);
    //TODO just handle intructions here
    // for (Instructions::const_iterator it = instructions.cbegin(); it != instructions.cend(); ++it) {
    //     lo_send(serverAddr, std::string("/gene/"+it->name()).c_str(), "f", it->expression().current);
    // }
//    receiveInstructions(conductor.instructions());
}

bool OSC::send(const std::string& path, const std::string& msg) {
    lo_send(serverAddr, path.c_str(), "s", msg.c_str());
    return true;
}

}  // namespace audiogene
