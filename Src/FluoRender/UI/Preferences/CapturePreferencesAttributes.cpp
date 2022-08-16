#include "CapturePreferencesAttributes.h"

#include <filesystem>
#include <iostream>
#include <sstream>

CapturePreferencesAttributes::CapturePreferencesAttributes()
{
}

// Returns the full path name for saving an image.
std::string CapturePreferencesAttributes::getFullName()
{
    std::string fullname = directory + "/" + filename;

    // If a family and index is needed. Find the last file with the base name.
    if(family)
    {
        constexpr int numDigits = 4;
        constexpr int numChars  = 2;

        int index = -1;

        // Iterate through all of the files in the specified directory.
        for (const auto & entry : std::filesystem::directory_iterator(this->directory))
        {
            if(!entry.is_regular_file())
                continue;

            std::string tmp(entry.path().filename().c_str());

            // Check if the file name matches the base file name.
            if(tmp.find_first_of(this->filename + "_") == 0)
            {
                // Remove the base file name and the underscore.
                tmp.erase(0, this->filename.size()+1);

                // Check for an extension match. The extension will be an underscore (removed above)
                // followed by four numbers 0000 plus a dot and format type extension "_0000.ext" or
                // four numbers 0000 an underscore and a letter plus a dot and format type extension "_0000_a.ext"
                if((tmp.find_first_of(".") == numDigits) ||
                   (tmp.find_first_of("_") == numDigits && tmp.find_first_of(".") == numDigits+numChars))
                {
                    std::string number = tmp;
                    number.erase(number.size()-numDigits);

                    if(tmp.find_first_of(".") == numDigits)
                    {
                        // Remove four digits and the dot, leaving just the format type extension.
                        tmp.erase(0, numDigits+1);
                    }
                    else if (tmp.find_first_of("_") == numDigits && tmp.find_first_of(".") == numDigits+numChars)
                    {
                        // Remove four digits, an underscore, a letter and the dot, leaving just the format type extension.
                        tmp.erase(0, numDigits+numChars+1);
                    }

                    // At this point the file and extension match the pattern so get the number.
                    if(tmp == this->extension)
                    {
                        if(index < std::stoi(number) + 1)
                            index = std::stoi(number) + 1;
                    }
                }
            }
        }

        // No index so start at zero.
        if(index < 0)
            index = 0;

        // Create a string with leading zeros and addit to the fullname.
        std::ostringstream oss;
        oss << "_" << std::setw(numDigits) << std::setfill('0') << index;

        fullname += oss.str();
    }

    // Add the file type extension.
    fullname += "." + extension;

    return fullname;
}
