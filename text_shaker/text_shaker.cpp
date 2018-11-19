#include <iostream>
#include <fstream>
#include <exception>
#include <unordered_map>
#include <random>
#include <cctype>

#include <boost/program_options.hpp>

#include "word_parser.hpp"

namespace po = boost::program_options;

struct Options {
    std::string filename;
    std::string output;
    std::uint64_t count;
};

static
std::optional<Options> parse_args(int argc, const char* const * argv) {
    Options opt;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("input,i", po::value(&opt.filename)->required(), "set input file(also 1 positional)")
        ("output,o", po::value(&opt.output), "set output file(default STDOUT)")
        ("words_num,w", po::value(&opt.count)->required(), "set words count(also 2 positional)")
    ;

    po::positional_options_description pos;
    pos.add("input", 1);
    pos.add("words_num", 1);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
        po::notify(vm);

        return opt;
    } catch (po::error& e) {
        std::cout << desc << std::endl;
        std::cerr << e.what() << std::endl;
        return {};
    }
}

static
std::optional<std::ifstream> open_file(const std::string& path) {
    std::ifstream stream(path);
    if(stream.fail()) {
        std::cerr << "File " << path << " don't exist" << std::endl;
        return {};
    }
    stream >> std::noskipws;
    return stream;
}

static
std::optional<std::ofstream> open_output_file(const std::string& path) {
    std::ofstream stream(path);
    if(stream.fail()) {
        std::cerr << "Can't create file " << path << std::endl;
        return {};
    }
    return stream;
}

int main(int argc, char** argv) {
    const auto opt = parse_args(argc, argv);
    if(!opt)
        return 1;

    auto file = open_file(opt->filename);
    if(!file)
        return 2;

    try {
        auto parser = WordParser(*file);
        parser.parse();
        // Select output stream
        if(opt->output.empty()) {
            parser.generate(opt->count, std::cout);
        } else {
            auto output = open_output_file(opt->output);
            if(!output)
                return 4;
            parser.generate(opt->count, *output);
        }
        return 0;
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 3;
    }
}
