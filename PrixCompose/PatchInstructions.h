
#include <string>
#include <vector>

#include "FileProperties.h"

#ifndef PATCHINSTRUCTIONS_H
#define PATCHINSTRUCTIONS_H


struct SongsObject
{

	std::vector<std::string> song_names;
	std::vector<bool> needs_assembly;
	//std::vector<bool> needs_compression;
	std::vector<int> song_numbers;


	SongsObject();

	void AddSong(std::string song_name, int song_number);
	void AddSongAssertAssembled(std::string song_name, int song_number);

};





struct PatchObject
{

	SongsObject* songs_object = nullptr;


	PatchObject();



	~PatchObject() { if (songs_object != nullptr) delete songs_object; };

};





#endif