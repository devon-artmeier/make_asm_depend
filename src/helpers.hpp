/*
	Copyright (c) 2025 Devon Artmeier

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

#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <algorithm>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

extern std::string StringToLower  (const std::string& str);
extern bool        CheckArgument  (const int argc, char* argv[], int& index, const std::string& option, bool ignore_case = false);
extern std::string ResolvePath    (const std::string& path);
extern std::string GetAbsolutePath(const std::string& path, const std::string& parent_path);
extern std::string GetRelativePath(const std::string& path);
extern void        AddSearchPath  (const std::string& path, const std::string& parent_path, std::unordered_set<std::string>& search_paths);

#endif // HELPERS_HPP
