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

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <random>
#include <memory>

#include "individual.hpp"

namespace audiogene {

// This is a preference
struct Criterion {
    double min;
    double max;
    double current;
};
// These are preferences
typedef std::map<AttributeName, Criterion> Criteria;

// Individuals are sorted based on how closely their instructions match the current preferences
// So when the audience updates a preference,
class AttributeSorter {
    Criteria mCriteria;

 public:
    void setCriteria(Criteria criteria) {
        mCriteria = criteria;
    }
    double get(const AttributeName& name) {
        return mCriteria[name].current;
    }

    // This is what gets called through main's input
    // It's taking in a preference that the audience is giving
    // This is then used to order the individuals
    // In the next generation, the individuals that most fit the overall preferences
    // will reproduce. The fittest of that will conduct the music for some period of time
    void updateCriterion(const AttributeName& name, bool increase) {
        Criterion c = mCriteria[name];
        if (increase) {
            mCriteria[name].current = std::min(c.max, c.current + 1);
        } else {
            mCriteria[name].current = std::max(c.min, c.current - 1);
        }
        spdlog::get("log")->info("Updated {} to {}", name, mCriteria[name].current);
    }
    // To sort individuals, go through each attribute and determine how close the value is to the criteria
    // the closer is it, the closer the result is to 1
    // normalize the sum of each attribute's proximity for each individual, and the winner is the one closer to zero
    bool operator() (const Individual& lhs, const Individual& rhs) {
        float lhsSimilarity = 0;
        float rhsSimilarity = 0;
        const uint8_t attributeCount = lhs.size();
        for (Individual::const_iterator it = lhs.cbegin(); it != lhs.cend(); ++it) {
            const double curVal = it->expression().current;
            const double ideal = mCriteria[it->name()].current;
            // const double ideal = mCriteria.get(it->name());
            float sim = 1 - (std::abs(ideal - curVal)) / (it->expression().max - it->expression().min);
            lhsSimilarity += sim;
        }

        for (Individual::const_iterator it = rhs.cbegin(); it != rhs.cend(); ++it) {
            const double curVal = it->expression().current;
            const double ideal = mCriteria[it->name()].current;
            // const double ideal = mCriteria.get(it->name());
            float sim = 1 - (std::abs(ideal - curVal)) / (it->expression().max - it->expression().min);
            rhsSimilarity += sim;
        }

        return (lhsSimilarity/attributeCount) > (rhsSimilarity/attributeCount);
    }
};

typedef std::set<Individual, AttributeSorter> Individuals;
class Population {
    std::shared_ptr<spdlog::logger> _logger;
    std::default_random_engine mRng;
    AttributeSorter mSorter;
    Individuals mIndividuals;

    const uint8_t mSize;
    uint8_t mGeneration;

    Criteria initializeCriteria(const Individual& seed);
    void initializePopulation(const Individual& seed);
    std::pair<Individual, Individual> getParents(Individuals fittest);
    Individual breed(std::pair<Individual, Individual> parents);
    void mutate(Individual child);

 public:
    Population();
    Population(const uint8_t n, const Individual& seed);
    ~Population() {};

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Population &obj) {
        os << "Population \n";
        for (const Individual &individual : obj.mIndividuals) {
            os << "\t" << individual << std::endl;
        }
        return os;
    }

    void nextGeneration(const uint8_t n);
    void updateCriterion(const AttributeName name);

    Individual fittest();
};

}  // namespace audiogene
