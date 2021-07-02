
#include <cstring>
#include <ctype.h> 
#include <iostream>

#include "PatchInstructions.h"
#include "FileProperties.h"
#include "Commands.h"
#include "SMKmusicdata.h"
#include "ErrorHandle.h"




SongsObject::SongsObject()
{
	song_names.clear();
	needs_assembly.clear();
}


void SongsObject::AddSong(std::string song_name, int song_number)
{
	try
	{
		if (!file_exists(song_name + unassembled_extension))
		{
			// throw error
			throw PrixException(song_name + unassembled_extension + " does not exist.");
		}

		size_t need_reassemble_ind = song_name.find(no_reassembly);
		if (need_reassemble_ind != std::string::npos)
		{
			needs_assembly.push_back(false);
			song_name.erase(need_reassemble_ind, strlen(no_reassembly));
		}
		else needs_assembly.push_back(true);


		song_names.push_back(song_name);
		song_numbers.push_back(song_number);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw; 
	}
}


void SongsObject::AddSongAssertAssembled(std::string song_name, int song_number)
{
	try
	{
		size_t need_reassemble_ind = song_name.find(no_reassembly);
		if (need_reassemble_ind != std::string::npos)
		{
			needs_assembly.push_back(false);
			song_name.erase(need_reassemble_ind, strlen(no_reassembly));
		}
		else needs_assembly.push_back(true);


		song_names.push_back(song_name);
		song_numbers.push_back(song_number);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw;
	}
}













PatchObject::PatchObject()
{

}