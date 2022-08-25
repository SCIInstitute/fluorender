#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include <vector>

class SourceAgent
{
public:
    SourceAgent() {};
    ~SourceAgent() {};

    void SaveProject(const std::wstring &filename) {};
    void OpenProject(const std::wstring &filename) {};
    void StartupLoad(const std::vector<std::wstring>& files, bool run_mov = false, bool with_imagej = false) {};
    void LoadVolume (const std::wstring& filename, bool withImageJ = false) {};
    void LoadVolumes(const std::vector<std::wstring>& files, bool withImageJ = false) {};
    void LoadMesh   (const std::wstring& filename) {};
    void LoadMeshes (const std::vector<std::wstring>& files) {};
};

#endif // SOURCEAGENT_H
