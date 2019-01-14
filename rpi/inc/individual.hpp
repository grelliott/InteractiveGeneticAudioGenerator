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

#include <iostream>
#include <vector>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "attribute.hpp"

namespace audiogen {

typedef typename std::vector<Attribute> Attributes;
class Individual {
    std::shared_ptr<spdlog::logger> _logger;
    Attributes mAttributes;
 public:
    Individual();
    Individual(const std::map<std::string, std::map<std::string, std::string>> attributes);
    Individual(const Attributes attributes);
    ~Individual() {};
    
    Attribute getAttribute(const std::string& name) const;
    typedef typename Attributes::iterator iterator;
    typedef typename Attributes::const_iterator const_iterator;
    inline iterator begin() noexcept { return mAttributes.begin(); }
    inline const_iterator cbegin() const noexcept { return mAttributes.cbegin(); }
    inline iterator end() noexcept { return mAttributes.end(); }
    inline const_iterator cend() const noexcept { return mAttributes.cend(); }
    inline uint8_t size() const noexcept { return mAttributes.size(); }

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Individual &obj) {
        os << "Individual \n";
        for (const Attribute &attribute: obj.mAttributes) {
            os << "\t" << attribute << "\n";
        }
        return os;
    }
};
    
}  // end audiogen
