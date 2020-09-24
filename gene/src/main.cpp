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

#include <gflags/gflags.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <future>

#include "performance.hpp"

// Command line argument flags
DEFINE_string(config, "", "Configuration for the genetics");  // NOLINT
DEFINE_string(log, "out.log", "Logfile path");  // NOLINT

int main(int argc, char* argv[]) {  // NOLINT
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    try {
        std::shared_ptr<spdlog::logger> logger;
        try {
            logger = spdlog::basic_logger_st("log", FLAGS_log, true);
            logger->set_level(spdlog::level::debug);
            logger->info("Logging initialized");
        } catch (const spdlog::spdlog_ex& e) {
            throw std::runtime_error(std::string("Log initialization failed: ") + e.what());
        }

        if (FLAGS_config.empty()) {
            throw std::runtime_error("Missing config argument");
        }

        logger->info("Loading config file {}", FLAGS_config);
        YAML::Node config = YAML::LoadFile(FLAGS_config);
        if (!config) {
            std::cerr << "Failed to load config file!" << std::endl;
            return -1;
        }

        audiogene::Performance performance(config);
        std::future<void> presentation = performance.play();
        presentation.get();
        return 0;
    } catch (const std::runtime_error& e) {
        std::cout << "Failed to start performance: " << e.what() << std::endl;
        return -1;
    }
}

