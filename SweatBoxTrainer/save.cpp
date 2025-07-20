#include "save.h"

#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "usermanager.h"
#include "basic_stream.h"
#include "filereader.h"
#include "guidialogue.h"
#include "airport.h"
#include "tools.h"
#include "globals.h"
#include "sim/simulation_context.h"

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
    // These would now be retrieved from a central settings object/manager
    buf.write_string(LAST_AGC_PATH.c_str());
    buf.write_string(LAST_APRT_DIR.c_str());
    buf.write_string(LAST_SCT_PATH.c_str());
    buf.end_frame_var_size_word();

    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    const auto full_path = std::filesystem::path(path).parent_path();

    std::fstream myFile(full_path.string() + "\\data.bin", std::ios::out | std::ios::binary);
    myFile.write(buf.data, buf.index);
    myFile.close();
}

void read_info()
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    auto full_path = std::filesystem::path(path).parent_path();

    std::fstream ifs(full_path.string() + "\\data.bin", std::ios::in | std::ios::binary | std::ios::ate);

    if (!ifs.is_open()) {
        AppendTextToConsole(L"No data.bin found. Skipping data load.");
        return;
    }

    std::fstream::pos_type size = ifs.tellg();
    if (size == 0) {
        ifs.close();
        AppendTextToConsole(L"data.bin is empty. Skipping data load.");
        return;
    }

    BasicStream buf = BasicStream(size);
    ifs.seekg(0, std::ios::beg);
    ifs.read(buf.data, size);
    ifs.close();
    buf.readable = size;

    // Create a single FileReader instance to reuse for all file loading.
    FileReader reader;

    while (buf.available() > 0)
    {
        int opcode = buf.read_unsigned_byte();
        int frame_size = buf.read_unsigned_short(); // All frames seem to be word-sized

        if (opcode == 1)
        {
            // This logic is for user identity, it remains the same.
            // Identity& id = *USER->getIdentity(); // This needs a valid USER object
            /*id.callsign = */buf.read_string();
            /*id.login_name = */buf.read_string();
            /*id.username = */buf.read_string();
            /*id.password = */buf.read_string();
            buf.read_unsigned_byte(); // Pilot rating
        }
        else if (opcode == 2)
        {
            if (frame_size > 0)
            {
                // Read the saved paths into the global settings variables
                LAST_AGC_PATH = buf.read_string();
                LAST_APRT_DIR = buf.read_string();
                LAST_SCT_PATH = buf.read_string();

                // --- THIS IS THE REFACTORED LOGIC ---
                if (!LAST_APRT_DIR.empty() && std::filesystem::exists(LAST_APRT_DIR) && std::filesystem::is_directory(LAST_APRT_DIR))
                {
                    AppendTextToConsole(L"Loading saved airport data from: " + s2ws(LAST_APRT_DIR));
                    for (const auto& entry : std::filesystem::directory_iterator(LAST_APRT_DIR))
                    {
                        if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".aprt")
                        {
                            // Use the new FileReader method
                            auto loaded_airport = reader.loadApt(entry.path().string());
                            auto& ctx = SimulationContext::instance();

                            if (loaded_airport) {
                                ctx.airports()[loaded_airport->icao] = std::move(loaded_airport);
                            }
                        }
                    }
                }

                // Future logic for reloading AGC or SCT files would go here,
                // using reader.loadAgc(LAST_AGC_PATH) or reader.loadSct(LAST_SCT_PATH).
            }
        }
    }

    AppendTextToConsole(L"Session data loaded.");
}