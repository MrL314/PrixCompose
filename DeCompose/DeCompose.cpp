

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <stdio.h>


#include "SMKmusicdata.h"
#include "Decompressor.h"
#include "Disassemble.h"
#include "FileProperties.h"
#include "ErrorHandle.h"




std::string inttohex(int n)
{
	std::stringstream ss;
	ss << std::hex << n;
	return ss.str();

}



std::string clean_ends(std::string in_string)
{
	std::string s(in_string);

	char bad_chars[] = " \n\r\t";

	//char bad_chars[4] = { 32, 9, 10, 13 };

	size_t start_pos = s.find_first_not_of(bad_chars);
	s.erase(0, start_pos);

	size_t end_pos = s.find_last_not_of(bad_chars);
	s.erase(end_pos + 1);

	/*
	while (s[0] == bad_chars[0] || s[0] == bad_chars[1] || s[0] == bad_chars[2] || s[0] == bad_chars[3])
	{
		s.erase(0, 1);
	}
	*/

	while (s[0] == '\"') s.erase(0, 1);

	while (s[s.length() - 1] == '\"') s.erase(s.length() - 1, 1);

	return s;
}



bool songNeedsCompression(int song_number) {
	if (song_number > 30 || song_number < 0) return false;
	else return is_compressed[song_number];
};


void PressAnyKey()
{
	std::cout << "Type EXIT, then press ENTER to exit program." << std::endl;
	char c;
	std::cin >> c;
}




void writeSongFile(std::vector<u8>& buffer, std::string song_name, bool is_compressed_song, u16 spc_address = 0xd000, u16 song_size = 0xffff, bool is_default = false)
{
	try
	{
		if (!is_default)
		{
			if (!file_exists(song_name + assembled_extension))
			{
				// throw error assembled file does not exist, if not a ripped file
				throw PrixException(song_name + assembled_extension + " does not exist! Cannot Decompose on non-existent file!");
			}
		}


		std::cout << "Creating " << song_name + unassembled_extension << std::endl;


		std::vector<u8> song_data;

		if (!is_compressed_song)
		{
			song_data.push_back((u8)(song_size & 255));
			song_data.push_back((u8)((song_size >> 8) & 255));
			song_data.push_back((u8)(spc_address & 255));
			song_data.push_back((u8)((spc_address >> 8) & 255));
		}
		

		for (int i = 0; i < song_size; i++)
		{
			song_data.push_back((u8)buffer[i]);
		}

		if (!is_compressed_song)
		{
			// remove tailing null space
			while (song_data.back() == 0) song_data.pop_back();
			song_data.push_back(0);

			song_data.push_back(0);
			song_data.push_back(0);
			song_data.push_back(0);
			song_data.push_back(8);
		}

		if (is_compressed_song)
		{
			// remove tailing END_DECOMP bytes
			while (song_data.back() == 0xff) song_data.pop_back();
			song_data.push_back(0xff);

			try
			{
				song_data = Decompress(song_data);
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





		for (int i = 0; i < 4; i++)
			song_data.pop_back();


		std::vector<std::string> disassembled_lines;


		MF_Disassembler mfd(song_data);

		mfd.DisassembleTo(disassembled_lines);

		//std::cout << create_file("songs/default/" + song_name + unassembled_extension) << std::endl;

		//create_file("songs/default/" + song_name + unassembled_extension);

		std::string new_file_name = create_file(song_name + unassembled_extension);

		//std::cout << new_file_name << std::endl;

		std::ofstream new_file;
		new_file.open(new_file_name, std::ofstream::out | std::ofstream::trunc);

		if (new_file.is_open())
		{

			for (auto line : disassembled_lines)
			{
				//std::cout << line << std::endl;
				new_file << line << std::endl;
			}

			new_file.close();
		}

		std::cout << "Successfully created " << song_name + unassembled_extension << std::endl;

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




int main(int argc, char* argv[])
{

	try
	{
		bool default_action = false;
		bool is_headered = true;

		bool is_compressed_song = false;

		std::string waste_in;
		std::vector<std::string> cmd_args;

		std::string FILE_NAME;
		std::string file_name;

		if (argc > 1)
		{
			for (int i = 0; i < argc; i++)
			{
				cmd_args.push_back(std::string(argv[i]));
			}

			if (argc > 2 && strcmp(argv[2], "-nh") == 0) is_headered = false;
		}
		else
		{
			std::cout << "Please input file you wish to DeCompose: ";

			std::cin >> FILE_NAME;

			FILE_NAME = clean_ends(FILE_NAME);

			file_name = FILE_NAME;

			if (FILE_NAME.compare("default") == 0 || FILE_NAME.compare("d") == 0)
			{
				default_action = true;
			}
			else
			{
				size_t ext_ind = file_name.find(assembled_extension);
				if (ext_ind != std::string::npos)
				{
					file_name.erase(ext_ind);
				}

				std::cout << "Is this file compressed? y or [n]: ";

				char cmprss;

				std::cin >> cmprss;

				if (cmprss == 'y')
				{
					is_compressed_song = true;
				}

			}

		}




		if (default_action)
		{
			std::cout << "Please input ROM file to rip default songs from: ";

			std::getline(std::cin, waste_in);

			std::string ROM_NAME;
			std::getline(std::cin, ROM_NAME);

			ROM_NAME = clean_ends(ROM_NAME);

			std::cout << "Does this file contain a header? [y] or n: ";

			char hdr;

			std::cin >> hdr;

			if (hdr == 'n')
			{
				is_headered = false;
			}



			std::string default_dir = create_dir("songs/default");

			std::fstream rom_file(ROM_NAME, std::ios_base::binary | std::ios::in | std::ios::out);

			if (rom_file)
			{
				rom_file.seekg(0, std::ios::end);
				int length = rom_file.tellg();
				rom_file.seekg(0, std::ios::beg);

				for (int i = 0; i < 31; i++)
				{
					int song_address = SONG_ROM_OFFSETS[i];
					int spc_address = SONG_SPC_OFFSETS[i];
					int song_size = DEFAULT_FILE_SIZES[i];
					is_compressed_song = is_compressed[i];

					std::string song_name = std::string(SONG_NAMES[i]) + "_default";

					


					rom_file.seekg(0, std::ios::beg);

					if (is_headered) song_address += 512;



					char* buffer = new char[song_size];

					rom_file.seekg(song_address);

					rom_file.read(buffer, song_size);


					std::vector<u8> SONG_BUFFER;

					for (int i = 0; i < song_size; i++)
					{
						SONG_BUFFER.push_back((u8)buffer[i]);
					}

					delete[] buffer;

					if (file_exists("songs/default/" + song_name + unassembled_extension))
					{
						remove(("songs/default/" + song_name + unassembled_extension).c_str());
					}

					try
					{
						// write the text file given the song file 
						writeSongFile(SONG_BUFFER, "songs/default/" + song_name, is_compressed_song, spc_address, song_size, true);

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
				rom_file.close();
			}
			else
			{
				// throw error when opening ROM file
				throw PrixException("Error opening " + ROM_NAME);
			}
		}
		else
		{
			if (!file_exists(FILE_NAME))
			{
				// throw error file doesnt exist
				throw PrixException(FILE_NAME + " does not exist!");
			}
			
			std::fstream song_file(FILE_NAME, std::ios_base::binary | std::ios::in | std::ios::out);

			if (song_file)
			{
				song_file.seekg(0, std::ios::end);
				int length = song_file.tellg();
				song_file.seekg(0, std::ios::beg);

					
				char* buffer = new char[length];

				song_file.read(buffer, length);

				std::vector<u8> SONG_BUFFER;

				for (int i = 0; i < length; i++)
				{
					SONG_BUFFER.push_back((u8)buffer[i]);
				}

				delete[] buffer;

				if (file_exists(file_name + unassembled_extension))
				{

					char c;
					std::cout << "This will overwrite the current " + file_name + unassembled_extension << std::endl;
					std::cout << "Overwrite? y or [n]: ";
					std::cin >> c;
					std::getline(std::cin, waste_in);

					if (c == 'y')
					{
						std::cout << "Overwriting " + file_name + unassembled_extension << std::endl;
						remove((file_name + unassembled_extension).c_str());
					}
				}

				// weird thing ask for SPC Address
				std::string HEX_IN;

				std::cout << "Please enter original SPC address or original mnemonic for file: ";
				std::getline(std::cin, HEX_IN);

				u16 addr = 32;

				try
				{
					addr = (u16)stoi(HEX_IN, 0, 16); // do i really need base 16?

				}
				catch (const std::invalid_argument&)
				{
					for (u16 i = 0; i < 31; i++)
					{
						if (strcmp(HEX_IN.c_str(), SONG_NAMES[i]) == 0)
						{
							addr = SONG_SPC_OFFSETS[i];
							break;
						}
					}
					if (addr == 32)
					{
						// throw error invalid argument for song offset
						throw PrixException("Invalid song offset: " + HEX_IN);
					}
				}



				try
				{
					writeSongFile(SONG_BUFFER, file_name, is_compressed_song, addr);
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
		}

	}
	catch (const PrixException& pe)
	{
		std::cout << pe.what() << std::endl;

		PressAnyKey();

		return EXIT_FAILURE;

	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;

		PressAnyKey();

		return EXIT_FAILURE;
	}

	PressAnyKey();


}