#pragma once
#include "back/simulation.h"
#include <iosfwd>
#include <string>

namespace model_detail {
struct CommandLineOptions {
    SimulationParameters parameters;
    bool help = false;
    bool quiet = false;
    bool show_orders = false;
    std::string report_file;
};
CommandLineOptions parse_command_line(int argc, char* argv[]);
void print_help(std::ostream& output);
void print_catalog(std::ostream& output);
void print_daily_report(std::ostream& output, const SimulationResult& result, bool show_orders);
void write_json_report(const std::string& path);
} // пространство имён model_detail
