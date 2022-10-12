#ifndef _DATASETAGENT_H_
#define _DATASETAGENT_H_

#include "InterfaceAgent.hpp"

#include "DatasetAttributes.h"

#include <string>
#include <vector>

namespace fluo
{
  class DatasetAgent : public InterfaceAgent
  {
  public:
    // A dataset is the fullname plus a type.
    typedef std::pair<std::string, DatasetAttributes::DatasetType> Dataset;
    typedef std::vector<Dataset> DatasetList;

    DatasetAgent() {};
    ~DatasetAgent() {};

    virtual void UpdateFui(const ValueCollection &names = {}) {};

    void setDatasetList(const DatasetList &datasetList) { mDatasetList = datasetList; };
    DatasetList getDatasetList() const { return mDatasetList; };

    void setActiveDataset(const std::string &activeDataset) { mActiveDataset = activeDataset; };
    std::string getActiveDataset() const { return mActiveDataset; };

  protected:
    virtual void setupInputs() {};

  private:
    DatasetList mDatasetList;
    std::string mActiveDataset;
  };
}

#endif // _DATASETAGENT_H_
