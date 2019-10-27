//
// Created by kwh44 on 10/27/19.
//

#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;


class Options {
public:
    std::vector<std::string> filenames;
    bool reverse_out = false;
    bool long_format = false;
    std::string sort_predicate;
};

void parse_options(int argc, char *argv[], Options &opts) {
    try {
        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "Help screen")
                ("long,l", "Long format output")
                ("sort", po::value<std::string>(), "Sort predicate output")
                ("reverse,r", "Reverse output order")
                ("filename", po::value<std::vector<std::string>>()->
                        multitoken()->zero_tokens()->composing(), "filename");

        po::positional_options_description pos_desc;
        pos_desc.add("filename", -1);
        po::command_line_parser parser{argc, argv};
        parser.options(desc).positional(pos_desc).allow_unregistered();
        po::parsed_options parsed_options = parser.run();
        po::variables_map vm;
        po::store(parsed_options, vm);
        if (vm.count("help")) std::cout << "myls [path|mask] [-l] [-h|--help] [--sort=U|S|t|X|D|s] [-r]\n";
        if (vm.count("filename"))
            std::for_each(vm["filename"].as<std::vector<std::string>>().begin(),
                          vm["filename"].as<std::vector<std::string>>().end(),
                          [&](auto &v) { opts.filenames.emplace_back(v); });
        if (vm.count("long")) opts.long_format = true;
        if (vm.count("reverse")) opts.reverse_out = true;
        if (vm.count("sort")) opts.sort_predicate = vm["sort"].as<std::string>();

    }
    catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
    }
}

int main(int argc, char *argv[], char *envp[]) {
    Options opts;
    parse_options(argc, argv, opts);

#ifdef PRINT_OPTIONS
    if (opts.filenames.empty() == false) {
        std::cout << "Filenames" << std::endl;
        for (auto &v: opts.filenames) {
            std::cout << v << std::endl;
        }
    }
    std::cout << "Reverse order" << std::endl;
    std::cout << opts.reverse_out << std::endl;

    std::cout << "Long format" << std::endl;
    std::cout << opts.long_format << std::endl;

    if (opts.sort_predicate.empty() == false) {
        std::cout << "Sort predicate" << std::endl;
        std::cout << opts.sort_predicate << std::endl;
    }
#endif

    return 0;
}

