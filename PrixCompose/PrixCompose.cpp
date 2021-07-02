

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctype.h> 
#include <limits>
#include <cctype>
#include <algorithm>


#include "PatchInstructions.h"
#include "FileProperties.h"
#include "SMKmusicdata.h"
#include "Commands.h"
#include "AssembleMusic.h"
#include "ErrorHandle.h"




std::string inttohex(int n)
{
	std::stringstream ss;
	ss << std::hex << n;
	return ss.str();

}


std::string chartohex(unsigned char c)
{
	std::stringstream ss;
	ss << std::hex << (int)c;
	std::string hex_string = ss.str();
	while (hex_string.length() < 2)
		hex_string = '0' + hex_string;
	return hex_string;
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
	std::cout << "Press ENTER to close program." << std::endl;
	std::string s;
	std::cin.clear();
	getline(std::cin, s);
}






int main(int argc, char* argv[])
{
	std::string instructions_file;

	bool is_headered = false;
	
	try
	{

		if (argc <= 1)
		{
			// throw error or just do default??
			if (argc == 1)
			{
				std::cout << "Please input patch instruction file name: ";

				//std::cin >> instructions_file;
				getline(std::cin, instructions_file);
				instructions_file = clean_ends(instructions_file);
				std::cin.clear();
			}
		}
		else
		{
			instructions_file = std::string(argv[1]);
			if (argc > 2)
			{
				if (strcmp(argv[2], "-nh") == 0)
				{
					is_headered = false;
				}
			}
		}



		if (!file_exists(instructions_file))
		{
			// throw error specified instructions file does not exist
			throw PrixException("File " + instructions_file + " does not exist.");
		}


		std::ifstream instructions(instructions_file);

		if (!instructions.is_open())
		{
			// throw error when opening file
			throw PrixException("Error when opening " + instructions_file);
		}

		PatchObject p = PatchObject();

		std::string ROM_FILE = "";
		SongsObject* s = nullptr;

		int line_number = 0;

		if (instructions)
		{
			std::string line;



			while (getline(instructions, line))
			{
				line = clean_ends(line);

				if (line.find("ROM_file") == 0)
				{
					if (ROM_FILE.length() != 0)
					{
						// throw error respecification of rom file
						throw PrixException("Cannot redefine ROM file.", line_number, line.c_str(), instructions_file.c_str());
					}

					line.erase(0, strlen("ROM_file"));
					line = clean_ends(line);

					if (line[0] == '=')
					{
						line.erase(0, 1);
						line = clean_ends(line);

						ROM_FILE = line;
					}
				}


				if (line.find("headered") != std::string::npos)
				{
					std::string raw_line = line;
					line.erase(0, strlen("headered"));
					line = clean_ends(line);

					if (line.length() != 0)
					{

						if (line[0] == '=')
						{
							line.erase(0, 1);
							line = clean_ends(line);

							std::string raw_control_text = line;

							std::for_each(line.begin(), line.end(), tolower);

							if (line.compare("true") == 0)
							{
								is_headered = true;
							}
							else if (line.compare("false") == 0)
							{
								is_headered = false;
							}
							else
							{
								// throw error unknown specification of header tag
								throw PrixException("Unknown specification of header tag: \"" + raw_control_text + "\"", line_number, raw_line, instructions_file);
							}
						}
						else
						{
							// throw error unknown characters after header tag
							throw PrixException("Unknown text after header tag: \"" + line + "\"", line_number, raw_line, instructions_file);
						}
					}
					else
					{
						is_headered = true;
					}

				}



				if (line.find("song_files") != std::string::npos)
				{
					if (s != nullptr)
					{
						// throw error cannot redefine song_files block
						throw PrixException("Cannot redefine song_files block.", line_number, line, instructions_file);
					}

					s = new SongsObject();

					bool contains_songs = false;
					if (line.find("{") != std::string::npos)
					{
						if (line.find("{") > line.find("song_files")) contains_songs = true;
					}
					else
					{
						getline(instructions, line);
						if (line.find("{") != std::string::npos) contains_songs = true;
					}

					// TODO: throw error if songs block not present?


					if (contains_songs)
					{
						int num_songs = 0;
						while (getline(instructions, line))
						{
							if (line.find("}") != std::string::npos) break;

							std::string songname = "";
							std::stringstream curr_line(line);

							std::getline(curr_line, songname, ':');

							

							int song_num = -1;

							songname = clean_ends(songname);

							if (songname.length() > 0)
							{

								try
								{
									song_num = stoi(songname, 0, 16); // do i really need base 16?

								}
								catch (const std::invalid_argument&)
								{
									for (int i = 0; i < 31; i++)
									{
										if (strcmp(songname.c_str(), SONG_NAMES[i]) == 0)
										{
											song_num = i;
											break;
										}
									}
									if (song_num == -1)
									{
										// throw error invalid argument for song offset
										throw PrixException("Invalid song offset: " + songname, line_number, line, instructions_file);
									}
								}


								getline(curr_line, songname);
								songname = clean_ends(songname);

								//std::cout << songname << std::endl;

								if (songname.length() > 1)
								{
									try
									{
										s->AddSong(songname, song_num);
										num_songs++;
									}
									catch (const PrixException& pe)
									{
										if (!file_exists(songname + assembled_extension))
										{
											throw PrixException(pe.get_msg(), line_number, line, instructions_file);
										}
										else
										{

											std::cout << "Warning: " << songname + unassembled_extension << " does not exist." << std::endl;
											std::cout << "Program will continue execution, but please keep a copy of the plaintext song file." << std::endl << std::endl;

											s->AddSongAssertAssembled(songname, song_num);
											num_songs++;
										}
									}
									catch (const std::exception& e)
									{
										throw PrixException("Unhandled error while adding songs.", -1, e.what());
									}
								}
							}
						}
					}
					else
					{
						delete s;
						s = nullptr;
					}
				}
				line_number++;

			}


			p.songs_object = s;


			if (ROM_FILE.length() == 0)
			{
				// throw error missing ROM_file specification
				throw PrixException("ROM_file not specified in patch instructions file.");
			}

			instructions.close();
		}


		std::fstream rom_file(ROM_FILE, std::ios_base::binary | std::ios::out | std::ios::in);

		if (rom_file.is_open())
		{
			rom_file.close();
		}
		else
		{
			// throw error could not open rom file
			throw PrixException("Error opening " + ROM_FILE);
		}


		///std::cout << ROM_FILE << std::endl;


		// here do passing check to see which files need to be assembled

		int song_ind = 0;
		for (auto song_name : p.songs_object->song_names)
		{
			//std::cout << song_name << std::endl;

			if (!p.songs_object->needs_assembly[song_ind])
			{
				if (!file_exists(song_name + assembled_extension))
				{
					p.songs_object->needs_assembly[song_ind] = true;
				}
			}


			if (p.songs_object->needs_assembly[song_ind])
			{
				if (file_exists(song_name + unassembled_extension))
				{

					//assemble the song here my dude
					AssembledFile f(song_name);

					//std::cout << f.file_name << std::endl;
					f.assemble_file();

					if (songNeedsCompression(p.songs_object->song_numbers[song_ind])) f.compress_file();
					else f.remove_header();

					f.writeFile();
				}
			}




			song_ind++;
		}





		// then upload the songs to the ROM

		//std::cout << "Rom_file: " << ROM_FILE << std::endl;

		rom_file.open(ROM_FILE, std::ios_base::binary | std::ios::out | std::ios::in);

		if (rom_file.is_open())
		{

			song_ind = 0;
			char* buffer = nullptr;
			int length;

			if (is_headered)
			{
				std::cout << ROM_FILE << " is specified as a headered ROM file. Proceeding with patching headered file." << std::endl << std::endl;
			}
			else
			{
				std::cout << ROM_FILE << " is specified as a non-headered ROM file. Proceeding with patching non-headered file." << std::endl << std::endl;
			}

			for (auto song : p.songs_object->song_names)
			{
				int song_offset = SONG_ROM_OFFSETS[p.songs_object->song_numbers[song_ind]];

				if (file_exists(song + assembled_extension))
				{

					std::ifstream song_file(song + assembled_extension, std::ios::binary);

					if (song_file.is_open())
					{

						song_file.seekg(0, std::ios::end);
						length = song_file.tellg();
						song_file.seekg(0, std::ios::beg);

						buffer = new char[length];

						song_file.read(buffer, length);

						song_file.close();


						int song_offset = SONG_ROM_OFFSETS[p.songs_object->song_numbers[song_ind]];

						int padded_length = DEFAULT_FILE_SIZES[p.songs_object->song_numbers[song_ind]];

						bool continue_patch = true;

						if (padded_length < length)
						{
							// raise warning new potential patch song larger than allotted space for song
							std::cout << "WARNING: Size of " << song + assembled_extension << " (0x" << inttohex(length) + ") larger than allocated space in ROM." << std::endl;
							std::cout << "Patching this file may have undesired consequences. Continue with patching " << song + assembled_extension << "? y or [n]: ";
							char c;
							std::cin >> c;
							if (tolower(c) == 'y') continue_patch = true;
							else continue_patch = false;

							std::cout << std::endl;

							//throw PrixException("Size of " + song + assembled_extension + " (" + std::to_string(length) + ") larger than allocated space in ROM.")
						}

						if (continue_patch)
						{
							std::cout << "Writing " << song + assembled_extension << " to " << ROM_FILE << " at $" << inttohex(song_offset) << std::endl;

							if (is_headered) song_offset += 512;


							rom_file.seekg(0, std::ios::beg);


							rom_file.seekg(song_offset);


							rom_file.write(buffer, length);

							/* for debugging
							for (int i = 0; i < length; i++)
							{
								std::cout << chartohex((unsigned char)buffer[i]) << " ";
							}
							*/


							char pad_character;

							if (songNeedsCompression(p.songs_object->song_numbers[song_ind]))
							{
								pad_character = 0xff;
							}
							else
							{
								pad_character = 0x00;
							}

							for (int i = 0; i < padded_length - length; i++)
							{
								rom_file.write(&pad_character, 1);
							}


							//std::cout << "Successfully patched " << song + assembled_extension << " (" << length << " bytes + " << (padded_length - length) << " padding)" << " to " << ROM_FILE << std::endl;
							std::cout << "Successfully patched " << song + assembled_extension << " to " << ROM_FILE << std::endl;
						}
						else
						{
							std::cout << "NOTICE: " << song + assembled_extension << " was not patched to " << ROM_FILE << std::endl << std::endl;
						}

						if (buffer != nullptr) delete[] buffer;
					}
					else
					{
						// throw error
						throw PrixException("Error opening " + song + assembled_extension);
					}
				}
				else
				{
					// throw error
					throw PrixException("Error opening " + song + assembled_extension);
				}

				song_ind++;
			}

			std::cout << std::endl << "Finished patching " << ROM_FILE << std::endl;


			rom_file.close();

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












