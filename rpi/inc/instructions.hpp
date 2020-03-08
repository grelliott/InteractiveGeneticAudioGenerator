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

#include "instruction.hpp"

/*
 * Instructions define the way the music should be played.
 *
 */
namespace audiogene {

using Instructions = std::vector<Instruction>;

// class Instructions {
// 	std::vector<Instruction> mInstructions;

//  public:
//  	Instructions();
//  	~Instructions() {}
//  	explicit Instructions(const std::map<std::string, std::map<std::string, std::string>> instructions);
//  	// some way for conductor to set
//  	// some way for musician to read

//  	// provide stream?

//  	// Provide Iterator interface
//     typedef typename std::vector<Instruction>::iterator iterator;
//     typedef typename std::vector<Instruction>::const_iterator const_iterator;
//     inline iterator begin() noexcept { return mInstructions.begin(); }
//     inline const_iterator cbegin() const noexcept { return mInstructions.cbegin(); }
//     inline iterator end() noexcept { return mInstructions.end(); }
//     inline const_iterator cend() const noexcept { return mInstructions.cend(); }
//     inline uint8_t size() const noexcept { return mInstructions.size(); }

//     template<typename OStream>
//     friend OStream &operator<<(OStream &os, const Instructions &obj) {
//         os << "Instructions: \n";
//         for (const Instruction &instruction : obj.mInstructions) {
//             os << "\t" << instruction << "\n";
//         }
//         return os;
//     }
// };

}  // namespace audiogene
