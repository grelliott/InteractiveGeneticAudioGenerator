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

#include "spi.hpp"

#include <wiringPiSPI.h>

#include <functional>

namespace audiogene {

SPI::SPI() {
	_logger = spdlog::get("log");
}

void SPI::prepare() {
	// std::thread spiListener; //member
    // Set up SPI
    int spiFd = wiringPiSPISetup(0, 500000);
    if (spiFd == -1) {
        _logger->warn("Failed to connect to SPI");
    } else {
        // make a new thread to listen to SPI
        spiListenerThread = std::thread(std::bind(&SPI::listener, this));

    }
}

void SPI::listener() {
    unsigned char buf[2];
    memset(buf, 0, 2);
    buf[0] = 0x80;
    // TODO change to some broadcast mechanism
    int stop = 0;
    // main loop
    while (stop == 0) {
        buf[0] = 0x80;
        wiringPiSPIDataRW(0, buf, 1);
        if (buf[0] != 0) {
            unsigned char data = buf[0];
            // loop through bits to see which are set
            for (size_t i = 0; i < sizeof(data) * 8; i++) {
                if (data & 1 << i) {
                    _logger->info("Received data from controller {}", i);
                    //TODO actually determine which preference was updated and update
                    preferenceUpdated({});
                }
            }
        }
        sleep(1);
    }

}

void SPI::preferenceUpdated(const Preference& preference) {
    (void)preference;
    //TODO actually do something with the updated preference
    // This is the audience, so it should notify the conductors somehow
}

}  // namespace audiogene
