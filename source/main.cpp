/*
	Copyright(c) 2024 Devon Artmeier

	Permission to use, copy, modify, and /or distribute this software
	for any purpose with or without fee is hereby granted.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
	WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIE
	WARRANTIES OF MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
	DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
	PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_set>

static std::string StringToLower(const std::string& str)
{
	std::string lower_str = str;
	std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) { return std::tolower(c); });
	return lower_str;
}

static bool CheckArgument(const int argc, char* argv[], int& index, const std::string& option, bool ignore_case = false)
{
	std::string option_copy = option;
	if (ignore_case) {
		option_copy = StringToLower(option);
	}

	if (strcmp(argv[index], ("-" + option_copy).c_str()) == 0) {
		if (++index >= argc) {
			throw std::runtime_error(("Missing parameter for \"-" + option + "\"").c_str());
		}
		return true;
	}
	return false;
}

static std::string GetSearchPath(const std::string& path, const std::string& parent_path)
{
	std::string search_path = std::filesystem::path(path).root_path().string();
	if (search_path.empty()) {
		search_path = parent_path + "/" + path;
	} else {
		search_path = path;
	}
	std::replace(search_path.begin(), search_path.end(), '\\', '/');
	return search_path;
}

static std::string GetRelativePath(const std::string& path)
{
	std::string new_path = std::filesystem::path(path).lexically_relative(std::filesystem::current_path().string()).string();
	std::replace(new_path.begin(), new_path.end(), '\\', '/');
	return new_path;
}

static void AnalyzeFile(const std::string& input_file, std::ofstream& output, const std::string& parent_path,
                        std::unordered_set<std::string>& dependencies, std::unordered_set<std::string>& search_paths,
                        std::unordered_set<std::string>& files_found)
{
	if (files_found.find(input_file) != files_found.end()) {
		throw std::runtime_error(("Multiple inclusions of \"" + GetRelativePath(input_file) + "\" found.").c_str());
	}
	files_found.insert(input_file);
	output << GetRelativePath(input_file) << " ";

	std::ifstream input(input_file, std::ios::in);
	if (!input.is_open()) {
		throw std::runtime_error(("Cannot open \"" + GetRelativePath(input_file) + "\" for reading.").c_str());
	}

	std::string line;
	while (std::getline(input, line)) {
		std::replace(line.begin(), line.end(), '\t', ' ');
		size_t start = line.find_first_not_of(' ');
		size_t end   = line.find_first_of(';');

		if (start != std::string::npos) {
			std::vector<std::string> split;
			line = line.substr(start, end - start);
			start = 0;

			while ((end = line.find(' ', start)) != std::string::npos) {
				std::string token = line.substr(start, end - start);
				if (start == 0) {
					size_t colon = token.find_first_of(':');
					if (colon != std::string::npos) {
						while (colon < token.length()) {
							if (token[colon] != ':') {
								break;
							}
							colon++;
						}

						if ((token.length() - colon) > 0) {
							token = token.substr(colon, token.length() - colon);
							split.push_back(token);
						}
					} else {
						split.push_back(token);
					}
				} else {
					split.push_back(token);
				}
				start = end + 1;
			}
			if (start < line.length()) {
				split.push_back(line.substr(start));
			}

			if (split.size() >= 2) {
				std::string directive = StringToLower(split[0]);
				std::string file      = split[1];

				if (file.length() > 2) {
					char start_quote = file[0];
					char end_quote   = file[file.length() - 1];
					if (start_quote == end_quote) {
						if (start_quote == '\'' || start_quote == '"') {
							file = file.substr(1, file.length() - 2);
						}
					}
				}

				if (directive.compare("incdir") == 0) {
					std::string path = GetSearchPath(file, parent_path);
					if (search_paths.find(path) == search_paths.end()) {
						search_paths.insert(path);
					}
				} else {
					std::string found_file = "";
					if (std::filesystem::exists(parent_path + "/" + file)) {
						found_file = parent_path + "/" + file;
					} else {
						for (const std::string& search_path : search_paths) {
							if (std::filesystem::exists(search_path + "/" + file)) {
								found_file = search_path + "/" + file;
								break;
							}
						}
					}

					if (!found_file.empty()) {
						found_file = std::filesystem::path(found_file).lexically_normal().string();
						std::replace(found_file.begin(), found_file.end(), '\\', '/');

						if (directive.compare("include") == 0) {
							AnalyzeFile(found_file, output, parent_path, dependencies, search_paths, files_found);
						} else if (directive.compare("incbin") == 0 || directive.compare("binclude") == 0) {
							output << GetRelativePath(found_file) << " ";
						}
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	std::string                     input_file = "";
	std::string                     output_file = "";
	std::string                     object_file = "";
	std::unordered_set<std::string> dependencies;
	std::unordered_set<std::string> search_paths;
	std::unordered_set<std::string> files_found;

	if (argc < 2) {
		std::cout << "Usage: make-asm-dependencies -o [output] [object file] <-i [search path]> [input file]" << std::endl << std::endl <<
		             "    -o [output] [object file] - Output file and object file" << std::endl <<
		             "    <-i [search path>         - Add search path" << std::endl <<
		             "    [input file]              - Input file" << std::endl << std::endl;
		return -1;
	}

	try {
		for (int i = 1; i < argc; i++) {
			if (CheckArgument(argc, argv, i, "o")) {
				if (!output_file.empty()) {
					throw std::runtime_error("Output file already defined.");
				}
				output_file = argv[i++];

				if (i >= argc) {
					throw std::runtime_error("Object file not defined.");
				}
				if (!object_file.empty()) {
					throw std::runtime_error("Object file already defined.");
				}
				object_file = argv[i];
				continue;
			}

			if (CheckArgument(argc, argv, i, "i")) {
				std::string path = GetSearchPath(argv[i], std::filesystem::current_path().string());
				if (search_paths.find(path) == search_paths.end()) {
					search_paths.insert(path);
				}
				continue;
			}

			if (!input_file.empty()) {
				throw std::runtime_error("Input file already defined.");
			}
			input_file = argv[i];
			std::replace(input_file.begin(), input_file.end(), '\\', '/');
		}

		if (input_file.empty()) {
			throw std::runtime_error("Input symbol file not defined.");
		}

		if (output_file.empty()) {
			throw std::runtime_error("Output symbol file not defined.");
		}

		std::ofstream output(output_file, std::ios::out);
		if (!output.is_open()) {
			throw std::runtime_error(("Cannot open \"" + output_file + "\" for writing.").c_str());
		}

		output << object_file << ": ";

		std::string parent_path  = std::filesystem::path(input_file).root_path().string();
		if (parent_path.empty()) {
			parent_path = std::filesystem::path(std::filesystem::current_path().string() + "/" + input_file).parent_path().string();
		} else {
			parent_path = std::filesystem::path(input_file).parent_path().string();
		}
		input_file = std::filesystem::path(input_file).filename().string();
		std::replace(parent_path.begin(), parent_path.end(), '\\', '/');

		AnalyzeFile(parent_path + "/" + input_file, output, parent_path, dependencies, search_paths, files_found);
		output << std::endl;
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}