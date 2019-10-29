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
    std::vector<bf::path> filenames;
    bool reverse_out = false;
    bool long_format = false;
    bool recursive = false;
    bool filetype = false;
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
                ("recursive,R", "Recursive iteration")
                ("filetype,F", "Print file types")
                ("filename", po::value<std::vector<bf::path>>()->
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
        if (vm.count("filename")) {
            auto files = vm["filename"].as<std::vector<bf::path>>();
            std::for_each(files.begin(),
                          files.end(),
                          [&](auto &v) { opts.filenames.emplace_back(v); });
        }
        if (vm.count("long")) opts.long_format = true;
        if (vm.count("reverse")) opts.reverse_out = true;
        if (vm.count("sort")) {
            opts.sort_predicate = vm["sort"].as<std::string>();
            for (auto &v: opts.sort_predicate) {
                if (v != 'U' || v != 'S' || v != 't' || v != 'X' || v != 'N' || v != 'D' || v != 's') {
                    exit(2);
                }
            }
        }
        if (vm.count("recursive")) opts.recursive = true;
        if (vm.count("filetype")) opts.filetype = true;
    }
    catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
        exit(1);
    }
}


void process_directory(bf::path &dir_path, Options &opts);

void process_files(std::vector<bf::path> &v, Options &opts, bool full_name);

int main(int argc, char *argv[], char *envp[]) {
    int return_code = 0;
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

    std::cout << "Recursive\n";
    std::cout << opts.recursive << std::endl;

    std::cout << "Filetype\n" << opts.filetype << std::endl;

    std::cout << "Long format" << std::endl;
    std::cout << opts.long_format << std::endl;

    if (opts.sort_predicate.empty() == false) {
        std::cout << "Sort predicate" << std::endl;
        std::cout << opts.sort_predicate << std::endl;
    }
#endif
    if (opts.filenames.empty()) {
        opts.filenames.emplace_back(bf::current_path().string());
    }

    std::vector<bf::path> pure_files;
    std::vector<bf::path> directories;

    for (auto &v: opts.filenames) {
        if (bf::exists(v)) {
            if (bf::is_directory(v)) directories.emplace_back(v);
            else pure_files.emplace_back(v);
        } else {
            std::cerr << "No such file or directory: " << v << std::endl;
            return_code = 1;
        }
    }
    process_files(pure_files, opts, true);
    for (auto &v: directories) process_directory(v, opts);
/*
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
*/
    return return_code;
}

void process_directory(bf::path &dir_path, Options &opts) {
    std::vector<bf::path> pure_files;
    std::vector<bf::path> directories;

    bf::directory_iterator it{dir_path};
    while (it != bf::directory_iterator{}) {
        if (bf::is_directory(it->path()) && opts.recursive) directories.emplace_back(it->path());
        pure_files.emplace_back(it->path());
        ++it;
    }
    std::cout << dir_path.filename().string() << ":" << std::endl;
    process_files(pure_files, opts, false);
    for (auto &v: directories) process_directory(v, opts);
}

void process_files(std::vector<bf::path> &files, Options &opts, bool full_name) {
    if (opts.long_format == false) {
        for (auto &v: files) {
            if (bf::is_directory(v)) std::cout << "/";
            if (full_name) std::cout <<  v.string();
            else std::cout << v.filename().string();
            std::cout << "    ";
        }
        std::cout << std::endl;
    }
}