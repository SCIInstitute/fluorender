#ifndef READERS_HPP
#define READERS_HPP

#include <memory>
#include <map>

#include <QString>
#include <functional>

#include <Formats/base_reader.hpp>
#include <Formats/tif_reader.hpp>
#include <Formats/nrrd_reader.hpp>
#include <Formats/oib_reader.hpp>
#include <Formats/oif_reader.hpp>
#include <Formats/lsm_reader.hpp>
//#include <Formats/pvxml_reader.hpp>
//#include <Formats/brkxml_reader.hpp>
//#include <Formats/msk_reader.hpp>
//#include <Formats/lbl_reader.hpp>

// Creates a reader from a map of readers and passes that reader to the 
// function requesting the memory. 
class Readers
{
  public:
    Readers(const QString &filenameExt)
    {
      reader = readers.at(filenameExt)();
    }

    auto returnReader() { return std::move(reader); }

    // for debugging purposes.
    void checkReader()  
    { 
      if(reader == nullptr) 
        std::cout << "The memory has been passed on." << std::endl; 
      else
        std::cout << "The memory still belongs to this class." << std::endl;
    }

  private:
    std::unique_ptr<BaseReader> reader;

    // Ensures that only 1 class gets created in memory instead of all classes.
    const std::map<QString,std::function<std::unique_ptr<BaseReader>()>> readers = {
      {"tiff", [](){ return std::make_unique<TIFReader>();}},
      {"tif",  [](){ return std::make_unique<TIFReader>();}},
      {"nrrd", [](){ return std::make_unique<NRRDReader>();}},
      {"oib",  [](){ return std::make_unique<OIBReader>();}},
      {"oif",  [](){ return std::make_unique<OIFReader>();}},
      {"lsm",  [](){ return std::make_unique<LSMReader>();}}
    };
};

#endif
