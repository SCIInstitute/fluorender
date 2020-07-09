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

// A Simple class to create a reader and hopefully move
// the ownership to whoever requests it. 
class Readers
{
  public:
    Readers(const QString &filenameExt)
    {
      reader = readers.at(filenameExt)();
    }

    auto returnReader() { return std::move(reader); }

  private:
    BaseReader* reader;
    const std::map<QString,std::function<BaseReader*()>> readers = {
      {".tiff", [](){ return new TIFReader();}},
      {".tif",  [](){ return new TIFReader();}},
      {".nrrd", [](){ return new NRRDReader();}},
      {".oib",  [](){ return new OIBReader();}},
      {".oif",  [](){ return new OIFReader();}},
      {".lsm",  [](){ return new LSMReader();}}
    };
};

#endif
