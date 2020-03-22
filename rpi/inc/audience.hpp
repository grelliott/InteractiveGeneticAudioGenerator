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

#include <map>

#include "preference.hpp"

namespace audiogene {

/*! An interface that each possible input source must implement
 *
 */
class Audience {
protected:
	Preferences mPreferences;

public:
	virtual bool prepare() = 0;

	void initializePreferences(const std::map<std::string, std::map<std::string, std::string>> attributes) {
		//attributeName => k/v (current)
		for (const std::pair<AttributeName, std::map<std::string, std::string>>& p : attributes) {
			AttributeName name = p.first;
			std::map<std::string, std::string> params = p.second;
			try {
				Preference p;
				p.current = std::stoi(params.at("current"));
				p.min = std::stoi(params.at("min"));
				p.max = std::stoi(params.at("max"));
				mPreferences[name] = p;
			} catch (const std::out_of_range& e) {
				continue;
			}
		}
	}

	void preferenceUpdated(const AttributeName name, const Preference& preference) {
		try {
			mPreferences.at(name) = preference;
		} catch (const std::out_of_range& e) {
			return;
		}
	}

	void preferenceUpdated(const AttributeName name, const int direction) {
		try {
			Preference p = mPreferences.at(name);
			p.current += direction;
			if (p.current < p.min) p.current = p.min;
			if (p.current > p.max) p.current = p.max;
			preferenceUpdated(name, p);
		} catch (const std::out_of_range& e) {
			return;
		}
	}

	Preferences preferences() {
		return mPreferences;
	}
};

}  // namespace audiogene
