#pragma once

#include "villas/exceptions.hpp"
#include "villas/log.hpp"
#include <filesystem>
#include <jansson.h>
#include <spdlog/logger.h>

using villas::ConfigError;

class IpLoader {
public:
  std::shared_ptr<spdlog::logger> logger;


  IpLoader(json_t* json_ips, const std::filesystem::path& searchPath): logger(villas::logging.get("IpParser")) {
    // Load IPs from a separate json file
    if (!json_is_string(json_ips)) {
      logger->debug("FPGA IP cores config item is not a string.");
      throw ConfigError(json_ips, "node-config-fpga-ips",
                        "FPGA IP cores config item is not a string.");
    }
    if (!searchPath.empty()) {
      std::filesystem::path json_ips_path =
          searchPath / json_string_value(json_ips);
      logger->debug("searching for FPGA IP cors config at {}", json_ips_path);
      json_ips = json_load_file(json_ips_path.c_str(), 0, nullptr);
    }
    if (json_ips == nullptr) {
      json_ips =
          json_load_file(json_string_value(json_ips), 0, nullptr);
      logger->debug("searching for FPGA IP cors config at {}",
                    json_string_value(json_ips));
      if (json_ips == nullptr) {
        throw ConfigError(json_ips, "node-config-fpga-ips",
                          "Failed to find FPGA IP cores config");
      }
    }

    if (not json_is_object(json_ips))
      throw ConfigError(json_ips, "node-config-fpga-ips",
                        "FPGA IP core list must be an object!");
  }
};
