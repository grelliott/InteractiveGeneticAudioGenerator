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

#include "midi.hpp"

#include <RtMidi.h>

#include <map>
#include <utility>
#include <algorithm>
#include <atomic>
#include <functional>
#include <sstream>

namespace audiogene {

MIDI::MIDI():
        MIDI("", {}) {
    // empty constructor
}

MIDI::MIDI(const std::string& name, const std::map<AttributeName, std::map<std::string, std::string>> mapping):
        _logger(spdlog::get("log")),
        _name(name),
        midiin(new RtMidiIn()) {
    // transform mapping
    for (const std::pair<AttributeName, std::map<std::string, std::string>>& p : mapping) {
        const AttributeName attributeName = p.first;
        const std::map<std::string, std::string> directionAndKey = p.second;
        for (const std::pair<std::string, std::string>& s : directionAndKey) {
            const std::string direction = s.first;
            std::string midiKey = s.second;
            _mapping[std::stoi(midiKey)] = std::pair<AttributeName, int>(attributeName, (direction == "up" ? 1 : -1));
        }
    }

    _logger->info("MIDI client created for device {}", _name);
}

bool MIDI::prepare() {
    uint16_t nPorts = midiin->getPortCount();
    if ( nPorts == 0 ) {
        _logger->error("No ports available!");
        return false;
    } else {
        _logger->info("{} ports available", nPorts);
    }

    if (!_name.empty()) {
        std::string portName;
        for (size_t i = 0; i < nPorts; i++) {
            try {
                portName = midiin->getPortName(i);
                _logger->debug("Testing port {} name = {}", i, portName);
                if (portName.rfind(_name, 0) == 0) {
                    // found our device
                    midiin->openPort(i);
                    break;
                }
            } catch (const RtMidiError& error) {
                error.printMessage();
            }
        }
    }

    // Default to first port if no port was opened yet
    if (!midiin->isPortOpen()) {
        midiin->openPort(0);
    }

    MIDI* me = this;
    midiin->setCallback([] (double timeStamp, std::vector<unsigned char> *message, void *userData) {
        (void)timeStamp;
        size_t messageSize = message->size();
        MIDI* that = static_cast<MIDI*>(userData);
        that->_logger->debug("Received MIDI message");
        if (messageSize > 0) {
            // we only care about note-off
            // TODO(grant) add some sort of buffer/back-off so that rapid bursts of a single event
            // don't all get handled
            if (message->at(0) == 0x80) {
                unsigned char key = message->at(1);
                that->_logger->debug("Note off for {}", key);
                if (that->_mapping.count(key) > 0) {
                    std::pair<AttributeName, int> attributeChange = that->_mapping[key];
                    that->_logger->info("Attribute {} changed {}", attributeChange.first, attributeChange.second);
                    that->preferenceUpdated(attributeChange.first, attributeChange.second);
                }
            }
        }
    }, me);

    _logger->info("MIDI client initialized");
    return true;
}

}  // namespace audiogene
