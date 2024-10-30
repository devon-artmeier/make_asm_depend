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

#include "helpers.hpp"

static bool CheckValidDirective(const std::string& directive)
{
	return directive.compare("incdir") == 0 || directive.compare("include") == 0 ||
	       directive.compare("incbin") == 0 || directive.compare("binclude") == 0;
}

static void AnalyzeFile(std::string input_file, std::ofstream& output, const std::string& parent_path,
                        std::unordered_set<std::string>& dependencies, std::unordered_set<std::string>& search_paths,
                        std::unordered_set<std::string>& files_found)
{
	std::ifstream input(input_file, std::ios::in);
	if (!input.is_open()) {
		throw std::runtime_error(("Cannot open \"" + GetRelativePath(input_file) + "\" for reading.").c_str());
	}

	files_found.insert(input_file);
	if (files_found.find(input_file) == files_found.end()) {
		output << " " << GetRelativePath(input_file);

		std::string line;
		while (std::getline(input, line)) {
			std::replace(line.begin(), line.end(), '\t', ' ');

			std::vector<std::string> split;
			size_t line_start = line.find_first_not_of(' ');
			size_t start      = 0;
			size_t end        = line.find_first_of(';');

			if (line_start != std::string::npos) {
				line = line.substr(line_start, end - line_start);

				while ((end = line.find(' ', start)) != std::string::npos) {
					std::string token = line.substr(start, end - start);
					if (start == 0) {
						// Check label
						size_t colon = token.find_first_of(':');
						if (colon != std::string::npos) {
							// Presence of colon means this is a label
							while (colon < token.length()) {
								if (token[colon] != ':') {
									break;
								}
								colon++;
							}

							if ((token.length() - colon) > 0) {
								// Label must actually exist
								token = token.substr(colon, token.length() - colon);
								split.push_back(token);
							}
						} else if (line_start > 0) {
							// Not label
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

					if (CheckValidDirective(directive)) {
						if (file.length() > 2) {
							// Detect quotation marks
							char start_quote = file[0];
							char end_quote   = file[file.length() - 1];
							if (start_quote == end_quote) {
								if (start_quote == '\'' || start_quote == '"') {
									file = file.substr(1, file.length() - 2);
								}
							}
						}

						if (directive.compare("incdir") == 0) {
							AddSearchPath(file, parent_path, search_paths);
						} else {
							std::string found_file = "";
							if (std::filesystem::exists(parent_path + "/" + file)) {
								found_file = parent_path + "/" + file;
							} else if (std::filesystem::exists(std::filesystem::current_path().string() + "/" + file)) {
								found_file = std::filesystem::current_path().string() + "/" + file;
							} else {
								for (const std::string& search_path : search_paths) {
									if (std::filesystem::exists(search_path + "/" + file)) {
										found_file = search_path + "/" + file;
										break;
									}
								}
							}
							found_file = ResolvePath(found_file);

							if (!found_file.empty()) {
								if (directive.compare("include") == 0) {
									AnalyzeFile(found_file, output, parent_path, dependencies, search_paths, files_found);
								} else if (directive.compare("incbin") == 0 || directive.compare("binclude") == 0) {
									output << " " << GetRelativePath(found_file);
								}
							}
						}
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	std::string                     input_file    = "";
	std::string                     output_file   = "";
	std::string                     object_file   = "";
	bool                            relative_path = false;
	std::unordered_set<std::string> dependencies;
	std::unordered_set<std::string> search_paths;
	std::unordered_set<std::string> files_found;

	if (argc < 2) {
		std::cout << "Usage: make_asm_depend -o [output] [object file] <-i [search path]> <-r> [input file]" << std::endl << std::endl <<
		             "    -o [output] [object file] - Output file and object file" << std::endl <<
		             "    <-i [search path>         - Add search path" << std::endl <<
		             "    <-r>                      - Use relative path finding" << std::endl <<
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

			if (StringToLower(argv[i]).compare("-r") == 0) {
				relative_path = true;
				continue;
			}

			if (CheckArgument(argc, argv, i, "i")) {
				AddSearchPath(argv[i], std::filesystem::current_path().string(), search_paths);
				continue;
			}

			if (!input_file.empty()) {
				throw std::runtime_error("Input file already defined.");
			}
			input_file = GetAbsolutePath(argv[i], std::filesystem::current_path().string());
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
		output << object_file << ":";

		std::string parent_path = std::filesystem::current_path().string();
		if (relative_path) {
			parent_path = std::filesystem::path(input_file).parent_path().string();
		}
		parent_path = ResolvePath(parent_path);

		AnalyzeFile(input_file, output, parent_path, dependencies, search_paths, files_found);
		output << std::endl;
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
