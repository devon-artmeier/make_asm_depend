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

std::string StringToLower(const std::string& str)
{
	std::string lower_str = str;
	std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) { return std::tolower(c); });
	return lower_str;
}

bool CheckArgument(const int argc, char* argv[], int& index, const std::string& option, bool ignore_case)
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

std::string ResolvePath(const std::string& path)
{
	if (std::filesystem::exists(path)) {
		std::string new_path = std::filesystem::path(path).lexically_normal().string();
		std::replace(new_path.begin(), new_path.end(), '\\', '/');
		return new_path;
	}
	return "";
}

std::string GetAbsolutePath(const std::string& path, const std::string& parent_path)
{
	std::string new_path = std::filesystem::path(path).root_path().string();
	if (new_path.empty()) {
		new_path = parent_path + "/" + path;
	} else {
		new_path = path;
	}
	return ResolvePath(new_path);
}

std::string GetRelativePath(const std::string& path)
{
	if (std::filesystem::exists(path)) {
		std::string new_path = std::filesystem::path(path).lexically_relative(std::filesystem::current_path().string()).string();
		std::replace(new_path.begin(), new_path.end(), '\\', '/');
		return new_path;
	}
	return "";
}

void AddSearchPath(const std::string& path, const std::string& parent_path, std::unordered_set<std::string>& search_paths)
{
	std::string search_path = std::filesystem::path(path).root_path().string();
	if (search_path.empty()) {
		search_path = parent_path + "/" + path;
	} else {
		search_path = path;
	}
	search_path = ResolvePath(search_path);

	if (!search_path.empty()) {
		if (search_paths.find(search_path) == search_paths.end()) {
			search_paths.insert(search_path);
		}
	}
}
