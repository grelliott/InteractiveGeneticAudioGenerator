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
		for (const std::pair<std::string, std::string>& s: directionAndKey) {
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

  	std::thread th([this] () {
  		std::vector<unsigned char> message;
	    size_t messageSize;

	    _logger->info("Listening for MIDI messages");
		while (true) {
			midiin->getMessage(&message);
			messageSize = message.size();

			if (messageSize > 0) {
				// we only care about note-off
				//TODO add some sort of buffer/back-off so that rapid bursts of a single event
				// don't all get handled
				if (message[0] == 0x80) {
					unsigned char key = message[1];
					_logger->debug("Note off for {}", key);
					if (_mapping.count(key) > 0) {
						std::pair<AttributeName, int> attributeChange = _mapping[key];
						_logger->info("Attribute {} changed {}", attributeChange.first, attributeChange.second);
						preferenceUpdated(attributeChange.first, attributeChange.second);
					}
				}
			}
	    	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	 	}
  	});
  	th.detach();

  	_logger->info("MIDI client initialized");
  	return true;
}

}  // namespace audiogene
