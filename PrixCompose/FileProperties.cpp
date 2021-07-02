
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <time.h>
#include <sstream>
#include <filesystem>

#include "FileProperties.h"

namespace fs = std::filesystem;



template <typename TP>
std::time_t to_time_t(TP tp)
{
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - TP::clock::now()
		+ std::chrono::system_clock::now());
	return std::chrono::system_clock::to_time_t(sctp);
}



std::string last_updated(std::string file_name)
{
	fs::path p = fs::current_path() / file_name;
	if (!fs::exists(p)) return std::string();

	std::ofstream(p.c_str());

	auto ftime = fs::last_write_time(p);
	std::time_t cftime = to_time_t(ftime);

	char buffer[70];

	struct tm newtime;
	errno_t err = localtime_s(&newtime, &cftime);

	std::strftime(buffer, sizeof buffer, "%%", &newtime);

	std::string return_string(buffer);


	return return_string;
}

bool has_been_modified(SimpleFileStruct& file, std::string last_modified)
{
	file.last_updated = last_updated(file.file_name);
	return (file.last_updated.compare(last_modified) != 0);
}

bool file_exists(std::string file_name)
{
	fs::path p = fs::current_path() / file_name;
	return fs::exists(p);
}


std::string full_path(std::string file_name)
{
	fs::path p = fs::current_path() / file_name;

	return p.string();
}

std::string create_dir(std::string file_path)
{

	fs::path p = fs::current_path() / file_path;
	fs::create_directories(p);

	return p.string();
}

std::string create_file(std::string file_name)
{
	fs::path p = fs::current_path() / file_name;
	if (!file_exists(file_name))
	{
		std::ofstream(p.c_str()).put('a');
	}

	return path_to_subdir_file(file_name);

}


std::string path_to_subdir_file(std::string file_path)
{
	int cur_path_len = fs::current_path().string().length();

	fs::path p = fs::current_path() / file_path;


	std::string parsed_path = p.string();

	parsed_path.erase(0, cur_path_len + 1);

	return parsed_path;
}
