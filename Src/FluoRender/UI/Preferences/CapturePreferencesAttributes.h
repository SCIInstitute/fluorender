#ifndef CAPTUREPREFERENCESATTRIBUTES_H
#define CAPTUREPREFERENCESATTRIBUTES_H

#include <string>

// Helper class to pass the Capture Preferences Attributes from the QvisCapturePreferencesDialog
// to the QvisViewWindow where the capture action occurs.
//
class CapturePreferencesAttributes
{
public:
    CapturePreferencesAttributes();

    // No get/set methods just direct access.
    std::string filename{"fluorender"};
    std::string directory{"."};
    std::string extension{"jpg"};

    bool family{true};
    bool embed{false};
    bool LZWCompression{false};
    bool saveAlphaChannel{false};
    bool saveFloatChannel{false};

    bool scaleOutputImage{false};
    double outputImageScale{1.0};

    // Returns the full path name for saving an image.
    std::string getFullName();
};

#endif // CAPTUREPREFERENCESATTRIBUTES_H
