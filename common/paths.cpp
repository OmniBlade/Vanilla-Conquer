// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection
#include "paths.h"
#include "debugstring.h"
#include "ini.h"
#include "rawfile.h"
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

PathsClass& PathsClass::Instance()
{
    static PathsClass _instance;

    return _instance;
}

void PathsClass::Init(const char* suffix, const char* ini_name, const char* cmd_arg)
{
    if (suffix != nullptr) {
        Suffix = suffix;
    }

    // Check the argv[0] arg assuming it was passed. This may be a symlink so should be checked.
    std::string cwd_path;

    if (cmd_arg != nullptr) {
        // If not absolute, should be relative to the working directory assuming it hasn't been changed yet.
        if (Is_Absolute(cmd_arg)) {
            cwd_path = cmd_arg;
        } else {
            char* cwd = getcwd(nullptr, 0);
            if (cwd != nullptr) {
                cwd_path = cwd;

                cwd_path += cmd_arg;
                free(cwd);
            }
        }

        // Remove exe path.
        cwd_path = cwd_path.substr(0, cwd_path.find_last_of("\\/"));
    }

    // Calls with unused returns to set the default variable values if not already set.
    Program_Path();
    Data_Path();
    User_Path();

    DBG_INFO("Searching the following paths for path config data:\n  argv: '%s'\n  binary: '%s'\n  default data: "
             "'%s'\n  default user: '%s'",
             cwd_path.c_str(),
             ProgramPath.c_str(),
             DataPath.c_str(),
             UserPath.c_str());

    RawFileClass file;
    bool use_cwd_path = false;
    bool use_program_path = false;

    // Check
    if (RawFileClass((cwd_path + SEP + ini_name).c_str()).Is_Available()) {
        file.Set_Name((cwd_path + SEP + ini_name).c_str());
        use_cwd_path = true;
    } else if (RawFileClass((ProgramPath + SEP + ini_name).c_str()).Is_Available()) {
        file.Set_Name((ProgramPath + SEP + ini_name).c_str());
        use_program_path = true;
    } else if (RawFileClass((UserPath + SEP + ini_name).c_str()).Is_Available()) {
        file.Set_Name((UserPath + SEP + ini_name).c_str());
    } else if (RawFileClass((DataPath + SEP + ini_name).c_str()).Is_Available()) {
        file.Set_Name((DataPath + SEP + ini_name).c_str());
    }

    INIClass ini;
    ini.Load(file);

    const char section[] = "Paths";
    char buffer[128]; // TODO max ini line size.

    // Even if the config was found with the binary, we still check to see if it gives use alternative paths.
    // If not, assume we are in portable mode and point the DataPath to ProgramPath.
    if (ini.Get_String(section, "DataPath", "", buffer, sizeof(buffer)) < sizeof(buffer) && buffer[0] != '\0') {
        DataPath = buffer;
    } else if (use_cwd_path) {
        DataPath = cwd_path;
    } else if (use_program_path) {
        DataPath = ProgramPath;
    }

    // Same goes for the UserPath.
    if (ini.Get_String(section, "UserPath", "", buffer, sizeof(buffer)) < sizeof(buffer) && buffer[0] != '\0') {
        UserPath = buffer;
    } else if (use_cwd_path) {
        UserPath = cwd_path;
    } else if (use_program_path) {
        UserPath = ProgramPath;
    }

    DBG_INFO("Read only data directory is set to '%s'", DataPath.c_str());
    DBG_INFO("Read/Write user data directory is set to '%s'", UserPath.c_str());
}

const char* PathsClass::Program_Path()
{
    if (ProgramPath.empty()) {
        Init_Program_Path();
    }

    return ProgramPath.c_str();
}

const char* PathsClass::Data_Path()
{
    if (DataPath.empty()) {
        Init_Data_Path();
    }

    return DataPath.c_str();
}

const char* PathsClass::User_Path()
{
    if (UserPath.empty()) {
        Init_User_Path();
    }

    return UserPath.c_str();
}