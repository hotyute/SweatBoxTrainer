#include "save.h"

#include <fstream>
#include <boost/dll.hpp>
#include <filesystem>

#include "usermanager.h"
#include "basic_stream.h"
#include "filereader.h"
#include "guidialogue.h"

void save_info()
{
	BasicStream buf = BasicStream(256);

	buf.create_frame_var_size_word(1);
	buf.write_string("");//"USER->getIdentity()->callsign.c_str());
	buf.write_string("");//USER->getIdentity()->login_name.c_str());
	buf.write_string("");//USER->getIdentity()->username.c_str());
	buf.write_string("");// USER->getIdentity()->password.c_str());
	buf.write_byte(0);// USER->getIdentity()->pilot_rating);
	buf.end_frame_var_size_word();

	buf.create_frame_var_size_word(2);
	buf.write_string(LAST_AGC_PATH.c_str());
	buf.write_string(LAST_APRT_DIR.c_str());
	buf.write_string(LAST_SCT_PATH.c_str());
	buf.end_frame_var_size_word();

	const auto full_path = boost::dll::program_location().parent_path();

	std::fstream myFile(full_path.string() + "\\data.bin", std::ios::out | std::ios::binary);
	myFile.write(buf.data, buf.index);
	myFile.close();
}

void read_info()
{
	auto full_path = boost::dll::program_location().parent_path();

	std::fstream ifs(full_path.string() + "\\data.bin", std::ios::in | std::ios::binary | std::ios::ate);

	if (ifs.is_open())
	{
		std::fstream::pos_type size = ifs.tellg();

		BasicStream buf = BasicStream(size);

		ifs.seekg(0, std::ios::beg);
		ifs.read(buf.data + buf.index, size);
		ifs.close();

		while (buf.available() != 0)
		{
			int opcode = buf.read_unsigned_byte();

			if (opcode == 1)
			{
				int size = buf.read_unsigned_short();
				//Identity& id = *USER->getIdentity();
				/*id.callsign = */buf.read_string();
				/*id.login_name = */buf.read_string();
				/*id.username = */buf.read_string();
				/*id.password = */buf.read_string();


				//USER->getIdentity()->controller_rating = buf.read_unsigned_byte();
				//USER->getIdentity()->controller_position = static_cast<POSITIONS>(buf.read_unsigned_byte());
			}
			else if (opcode == 2)
			{
				int size = buf.read_unsigned_short();
				if (size > 0)
				{
					std::string file_path = buf.read_string();
					std::string aprt_dir = buf.read_string();
					std::string pof_path = buf.read_string();
					if (!aprt_dir.empty())
					{
						if (std::filesystem::exists(aprt_dir) && std::filesystem::is_directory(aprt_dir))
						{
							for (const auto& entry : std::filesystem::directory_iterator(aprt_dir))
							{
								if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".aprt")
									LoadAPT(entry.path().string());
							}
						}
					}
					//if (!file_path.empty())
					//	parseCpfFile(cpf_file_path, filerdr.collisionPaths);
				}
			}
		}

		AppendTextToConsole(L"Data Loaded.");
		//do something with data
		return;
	}

	AppendTextToConsole(L"Failed to load Data.");
}
