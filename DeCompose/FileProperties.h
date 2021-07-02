

#include <string>

#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H



//std::string asctime_to_condensedtime(std::string asctime, std::string last_modified);




struct SimpleFileStruct
{
	std::string file_name;

	std::string last_updated;

};



std::string last_updated(std::string filename);

bool has_been_modified(SimpleFileStruct& file, std::string last_modified);

std::string full_path(std::string filename);

bool file_exists(std::string file_name);


std::string create_dir(std::string file_path);

std::string create_file(std::string file_name);

std::string path_to_subdir_file(std::string file_path);


#endif