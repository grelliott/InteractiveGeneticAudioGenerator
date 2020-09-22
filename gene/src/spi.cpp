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

auto SPI::prepare() -> bool {
    // Set up SPI
    int spiFd = wiringPiSPISetup(SPI_CHANNEL, MAX_SPEED);
    if (spiFd == -1) {
        _logger->warn("Failed to connect to SPI");
        return false;
    }

    // make a new thread to listen to SPI
    spiListenerThread = std::thread([this] () {
        std::array<unsigned char, 1> buf{ {SIGNAL} };
        // TODO(grant) change to some broadcast mechanism
        int stop = 0;
        // main loop
        while (stop == 0) {
            buf[0] = SIGNAL;
            wiringPiSPIDataRW(SPI_CHANNEL, buf.data(), 1);
            if (buf.at(0) != 0) {
                unsigned char data = buf.at(0);
                // loop through bits to see which are set
                for (size_t i = 0; i < sizeof(data) * BYTE_SIZE; i++) {
                    if ((data & 1u << i) != 0) {
                        _logger->info("Received data from controller {}", i);
                        // TODO(grant) actually determine which preference was updated and update
                        preferenceUpdated("", {});
                    }
                }
            }
            sleep(1);
        }
    });
    return true;
}

}  // namespace audiogene
