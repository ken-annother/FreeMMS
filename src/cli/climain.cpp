#include "climain.h"
#include <iostream>

#include <boost/program_options.hpp>
#include "MMSEngine.h"

using namespace std;
using namespace boost::program_options;

static inline void print_version(const char *exeName) {
    std::cout << exeName << " v0.9.0 Compiled on " << __DATE__ << std::endl;
}


int decodeMMS(int argc, const char **argv) {
    char *optionDescStr = (char *) malloc(strlen(argv[0]) + strlen(" options") + 1);
    memset(optionDescStr, 0, strlen(argv[0]) + strlen(" options") + 1);
    strcat(optionDescStr, argv[0]);
    strcat(optionDescStr, " options");

    options_description desc(optionDescStr);
    free(optionDescStr);

    bool withDir;

    desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "produce version message")
            ("output,o", value<string>(), "set output file or directory")
            ("with-dir", value<bool>(&withDir)->default_value(false), "whether output mms part body to directory")
            ("input-file", value<string>(), "input file");

    positional_options_description p;
    p.add("input-file", -1);

    variables_map vm;

    try {
        store(command_line_parser(argc, argv)
                      .options(desc)
                      .positional(p)
                      .run(),
              vm);
        notify(vm);
    } catch (...) {
        cout << desc << endl;
        return -1;
    }


    if (vm.count("help") || argc == 1) {
        cout << desc << endl;
        return 0;
    }

    if (vm.count("version")) {
        print_version(argv[0]);
        return 0;
    }

    string output;
    if (vm.count("output")) {
        output = vm["output"].as<string>();
    }

    if (withDir && output.empty()) {
        cout << desc << endl;
        return -1;
    }

    string input;
    if (vm.count("input-file")) {
        input = vm["input-file"].as<string>();
    }

    MMSEngine engine;
    if (withDir) {
        engine.convert2PlainDirectory(input, output);
        cout << "Success" << endl;
    } else {
        cout << engine.convert2Plain(input) << endl;
    }

    return 0;
}


int encodeMMS(int argc, const char **argv) {
    std::cout << "encode mms had not implementation yet" << std::endl;
    return 0;
}


int pre_main(int argc, const char **argv, int state) {
    if (state == 1) {
        argv[0] = "mms2plain";
        return decodeMMS(argc, argv);
    } else {
        argv[0] = "plain2mms";
        return encodeMMS(argc, argv);
    }
}
