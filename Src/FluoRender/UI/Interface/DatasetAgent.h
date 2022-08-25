#ifndef DATASETAGENT_H
#define DATASETAGENT_H

#include <string>
#include <vector>

#include "DatasetAttributes.h"

class DatasetAgent
{
public:
    // A dataset is the fullname plus a type.
    typedef std::pair<std::string, DatasetAttributes::DatasetType> Dataset;
    typedef std::vector<Dataset> DatasetList;

    DatasetAgent() {};
    ~DatasetAgent() {};

    void setDatasetList(const DatasetList &datasetList) { mDatasetList = datasetList; };
    DatasetList getDatasetList() const { return mDatasetList; };

    void setActiveDataset(const std::string &activeDataset) { mActiveDataset = activeDataset; };
    std::string getActiveDataset() const { return mActiveDataset; };

private:
    DatasetList mDatasetList;
    std::string mActiveDataset;
};

#endif // DATASETAGENT_H
