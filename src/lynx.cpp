#include "lynx.h"
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#pragma warning(disable: 4244)

float CLynx::AngleMod(float a)
{
    // from quake 2
    return (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
}

std::string CLynx::StripFileExtension(std::string path)
{
    size_t pos;

    pos = path.rfind('.');
    if(pos == std::string::npos)
        return path;

    return path.substr(0, pos);
}

std::string CLynx::ChangeFileExtension(std::string path, std::string newext)
{
    return StripFileExtension(path) + "." + newext;
}

std::string CLynx::GetDirectory(std::string path)
{
    size_t pos;

    pos = path.find_last_of("/\\");
    if(pos == std::string::npos)
        return path;

    return path.substr(0, pos+1);
}

std::string CLynx::FloatToString(float f, int precision)
{
   std::ostringstream o;
   o.precision(precision);
   if(!(o << f))
   {
       assert(0); // Unable to convert float to string?
       return "";
   }
   return o.str();
}

int CLynx::random(int min, int max)
{
    return min + (int)((double)rand() / (RAND_MAX / (max - min + 1) + 1));
}

std::string CLynx::GetRandNumInStr(const char* str, unsigned int maxnumber)
{
    char tmpstr[512];

    assert(strlen(str) < sizeof(tmpstr));
    sprintf(tmpstr, str, CLynx::random(1, maxnumber));
    return std::string(tmpstr);
}

