//
// Created by kwh44 on 10/27/19.
//

#include <vector>
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>
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
                if (v != 'U' && v != 'S' && v != 't' && v != 'X' && v != 'N' && v != 'D' && v != 's') {
                    std::cerr << "Wrong sorting predicate." << std::endl;
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


void process_directory(bf::path &dir_path, Options &opts, bool print_dirname = true);

void process_files(std::vector<bf::path> &v, Options &opts, bool full_name);

int main(int argc, char *argv[], char *envp[]) {
    int return_code = 0;
    Options opts;
    parse_options(argc, argv, opts);

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
    if (directories.size() == 1 && pure_files.empty()) process_directory(directories[0], opts, false);
    else {
        for (auto &v: directories) process_directory(v, opts);
    }
    return return_code;
}

void process_directory(bf::path &dir_path, Options &opts, bool print_dirname) {
    std::vector<bf::path> pure_files;
    std::vector<bf::path> directories;

    bf::directory_iterator it{dir_path};
    while (it != bf::directory_iterator{}) {
        if (bf::is_directory(it->path()) && opts.recursive) directories.emplace_back(it->path());
        pure_files.emplace_back(it->path());
        ++it;
    }
    if (print_dirname)
        std::cout << dir_path.string() << ":" << std::endl;
    process_files(pure_files, opts, false);
    for (auto &v: directories) process_directory(v, opts);
}

class FileStat {
public:
    bf::file_status status;
    bf::file_type type;
    bf::perms perms;
    std::time_t last_modified_time;
    std::uintmax_t file_size;
    bf::path path;

    explicit FileStat(bf::path &v) {
        path = v;
        status = bf::status(v);
        perms = status.permissions();
        type = status.type();
        last_modified_time = bf::last_write_time(v);
        boost::system::error_code ec;
        file_size = bf::file_size(v, ec);
    }
};

bool is_special_file(FileStat &file) {
    if (file.type == bf::file_type::directory_file) return false;
    if (file.perms & S_IEXEC) return true;
    return (file.type != bf::file_type::regular_file);
}

typedef std::function<bool(FileStat &a, FileStat &b)> predicate_t;

void sort_paths(std::vector<FileStat> &files, Options &opts) {
    static std::map<char, predicate_t> predicates = {
            {'S', [&opts](FileStat &a, FileStat &b) { return (a.file_size < b.file_size) != opts.reverse_out; }},
            {'t', [&opts](FileStat &a, FileStat &b) {
                return (a.last_modified_time < b.last_modified_time) != opts.reverse_out;
            }},
            {'X', [&opts](FileStat &a, FileStat &b) {
                return (a.path.extension() < b.path.extension()) != opts.reverse_out;
            }},
            {'N', [&opts](FileStat &a, FileStat &b) {
                return (a.path.filename() < b.path.filename()) != opts.reverse_out;
            }},
            {'D', [&opts](FileStat &a, FileStat &b) {
                return (a.type == bf::file_type::directory_file && b.type != bf::file_type::directory_file) !=
                       opts.reverse_out;
            }},
            {'s', [&opts](FileStat &a, FileStat &b) {
                return (!is_special_file(a) && is_special_file(b)) != opts.reverse_out;
            }}
    };
    for (auto &c: opts.sort_predicate) {
        std::sort(files.begin(), files.end(), predicates[c]);
    }
}

std::string type_sign(FileStat &v) {
    if (v.type == bf::file_type::directory_file) return "/";
    if (v.perms & S_IEXEC) return "*";
    if (v.type == bf::file_type::regular_file) return "";
    if (v.type == bf::file_type::socket_file) return "=";
    if (v.type == bf::file_type::symlink_file) return "@";
    if (v.type == bf::file_type::fifo_file) return "|";
    return "?";
}

void process_files(std::vector<bf::path> &files, Options &opts, bool full_name) {
    std::vector<FileStat> filestats;
    for (auto &v: files) filestats.emplace_back(FileStat(v));

    if (opts.sort_predicate.empty() == false) sort_paths(filestats, opts);

    for (auto &v: filestats) {
        if (opts.filetype || v.type == bf::file_type::directory_file) std::cout << type_sign(v);
        if (full_name) std::cout << v.path.string();
        else std::cout << v.path.filename().string();
        std::cout << "    ";
        if (opts.long_format) std::cout << v.file_size << "    " << std::ctime(&v.last_modified_time);
    }
    std::cout << std::endl;
}