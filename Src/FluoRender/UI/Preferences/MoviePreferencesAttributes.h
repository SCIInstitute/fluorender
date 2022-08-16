#ifndef MOVIEPREFERENCESATTRIBUTES_H
#define MOVIEPREFERENCESATTRIBUTES_H

#include <string>

class MoviePreferencesAttributes
{
public:
    enum OutputType
    {
        TIFF,
        MOV,
    };

    MoviePreferencesAttributes();

    // No get/set methods just direct access.
    std::string filename{"fluorender"};
    std::string directory{"."};
    std::string extension{"tiff"};
    OutputType fileType{TIFF};

    bool embed{false};

    bool LZWCompression{false};
    bool saveAlphaChannel{false};
    bool saveFloatChannel{false};

    double bitrate{20.0};

    bool scaleOutputImage{false};
    double outputImageScale{1.0};
};

#endif // MOVIEPREFERENCESATTRIBUTES_H
