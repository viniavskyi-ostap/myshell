//
// Created by kwh44 on 10/27/19.
//

#include <vector>
#include <iostream>
#include <string>
#include <ctime>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

class Options {
public:
    std::vector<std::string> filenames;
    bool reverse_out = false;
    bool long_format = false;
    std::string sort_predicate;
};

//void

void parse_options(int argc, char *argv[], Options &opts) {
    try {
        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "Help screen")
                ("long,l", "Long format output")
                ("sort", po::value<std::string>(), "Sort predicate output")
                ("reverse,r", "Reverse output order")
                ("recursive,R", "Recursive iteration")
                ("filetype,F", "Print file types")
                ("filename", po::value<std::vector<std::string>>()->
                        multitoken()->zero_tokens()->composing(), "filename");

        po::positional_options_description pos_desc;
        pos_desc.add("filename", -1);
        po::command_line_parser parser{argc, argv};
        parser.options(desc).positional(pos_desc).allow_unregistered();
        po::parsed_options parsed_options = parser.run();
        po::variables_map vm;
        po::store(parsed_options, vm);
        if (vm.count("help")) {
            std::cout << "myls [path|mask] [-l] [-h|--help] [--sort=U|S|t|X|D|s] [-r]\n";
            exit(0);
        }
        if (vm.count("filename"))
            std::for_each(vm["filename"].as<std::vector<std::string>>().begin(),
                          vm["filename"].as<std::vector<std::string>>().end(),
                          [&](auto &v) { opts.filenames.emplace_back(v); });
        if (vm.count("long")) opts.long_format = true;
        if (vm.count("reverse")) opts.reverse_out = true;
        if (vm.count("sort")) opts.sort_predicate = vm["sort"].as<std::string>(); // TODO: check for correct value
    }
    catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
        exit(1);
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

    if (opts.filenames.empty() == false) {
        std::vector<std::string> filenames;

        for (auto &v: opts.filenames) {
            if (bf::exists(v)) {
//                bf::path::
//                auto status = bf::status(v);
//                std::cout << "Type is " << status.type() << std::endl;
//                std::cout << "Permissions are " << status.permissions() << std::endl;
//                std::time_t t = bf::last_write_time(v);
//                auto last_modified = std::ctime(&t);
//                std::cout << "Last write time is " << last_modified << std::endl;
//                boost::system::error_code ec;
//                boost::uintmax_t fsize = bf::file_size(v, ec);
//                if (!ec) std::cout << "File size is " << fsize << std::endl;
//                else std::cout << "File_size failed" << std::endl;
            } else std::cout << "No file or directory found: " << v << std::endl;
        }
    }
    return 0;
}
