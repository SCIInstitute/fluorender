#ifndef DATASETATTRIBUTES_H
#define DATASETATTRIBUTES_H

class DatasetAttributes
{
public:
    enum DatasetType {
        Unknown        = 0x0000,
        ActiveDatasets = 0x0001,
        ViewWindow     = 0x0002,
        VolumeGroup    = 0x0004,
        Volume         = 0x0008,
        MeshGroup      = 0x0010,
        Mesh           = 0x0020,
        Annotations    = 0x0040,
    };

    DatasetAttributes();
};

#endif // DATASETATTRIBUTES_H
