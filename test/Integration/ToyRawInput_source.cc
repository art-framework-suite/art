// ------------------------------------------------------------
//
// ToyRawInput is a RawInputSource that pretends to reconstitute several
// products. It exercises the ReaderSource template.
//
// ------------------------------------------------------------

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/ReaderSource.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include "cpp0x/memory"

#include <cstdio>

namespace
{
  // Helper function to throw an exception with the appropriate text.
  void throw_exception_from(const char* funcname)
  {
    throw art::Exception(art::errors::OtherArt)
      << "Expected exception from ToyRawInputSource::"
      << funcname
      << '\n';
  }
}

using fhicl::ParameterSet;
using std::string;
using std::vector;
using std::shared_ptr;
using namespace art;


// Our simulated input file format is:
// A parameter in a parameter set, which contains a vector of vector of int.
// Each inner vector is a triplet of run/subrun/event number.
//   -1 means no new item of that type
//

namespace arttest
{

  //
  // ToyFile is the sort of class that experimenters who make use of
  // ReaderSource must write.
  //
  class ToyFile
  {
  public:
    ToyFile(fhicl::ParameterSet const& ps,
            art::ProductRegistryHelper& help,
            art::PrincipalMaker const& pm);

    void closeCurrentFile();

    void readFile(std::string const &name,
                  art::FileBlock*& fb);

    bool readNext(art::RunPrincipal* const& inR,
                  art::SubRunPrincipal* const& inSR,
                  art::RunPrincipal*& outR,
                  art::SubRunPrincipal*& outSR,
                  art::EventPrincipal*& outE);

  private:
    typedef std::vector<std::vector<int> > vv_t;
    typedef vv_t::const_iterator iter;

    art::PrincipalMaker const & pm_;

    iter    current_;
    iter    end_;
    ParameterSet data_;
    vv_t fileData_;
    string currentFilename_;
    bool throw_on_construction_;
    bool throw_on_closeCurrentFile_;
    bool throw_on_readNext_;
    bool throw_on_readFile_;
  };

  ToyFile::ToyFile(fhicl::ParameterSet const& ps,
                   art::ProductRegistryHelper& helper,
                   art::PrincipalMaker const &pm) :
    pm_(pm),
    current_(),
    end_(),
    data_(ps),
    fileData_(),
    currentFilename_(),
    throw_on_construction_(ps.get<bool>("throw_on_construction", false)),
    throw_on_closeCurrentFile_(ps.get<bool>("throw_on_closeCurrentFile", false)),
    throw_on_readNext_(ps.get<bool>("throw_on_readNext", false)),
    throw_on_readFile_(ps.get<bool>("throw_on_construction", false))
  {
    if (throw_on_construction_) throw_exception_from("ToyFile");
    helper.reconstitutes<int, InEvent>("m1");
    helper.reconstitutes<double, InSubRun>("s1");
    helper.reconstitutes<double, InRun>("r1");
    helper.reconstitutes<bool, InEvent>("m2", "a");
    helper.reconstitutes<bool, InEvent>("m2", "b");
  }

  void ToyFile::closeCurrentFile()
  {
    if (throw_on_closeCurrentFile_) throw_exception_from("closeCurrentFile");
    fileData_.clear();
    current_ = iter();
    end_ = iter();
  }

  bool ToyFile::readNext(art::RunPrincipal* const& inR,
                         art::SubRunPrincipal* const& inSR,
                         art::RunPrincipal*& outR,
                         art::SubRunPrincipal*& outSR,
                         art::EventPrincipal*& outE)
  {
    if (throw_on_readNext_) throw_exception_from("readNext");
    // Have we any more to read?
    if (current_ == end_) return false;

    if (data_.get<bool>("returnTrueWithoutNewFault", false)) return true;

    bool readSomething = false;

    if ((*current_)[0] != -1) // New run
      {
        Timestamp runstart; // current time?
        outR = pm_.makeRunPrincipal((*current_)[0],  // run number
                                    runstart);     // starting time
        put_product_in_principal(std::auto_ptr<double>(new double(76.5)),
                                 *outR,
                                 "r1");
         readSomething = true;
      }
    if ((*current_)[1] != -1) // New subrun
      {
        assert(outR || inR); // Must have one or the other.
        SubRunID newSRID;
        if (data_.get<bool>("newRunSameSubRunFault", false) && inSR)
          {
            newSRID = SubRunID(inSR->id());
          }
        else
          {
            newSRID = SubRunID(outR?outR->run():inR->run(),
                               (*current_)[1]);
          }
        Timestamp runstart; // current time?
        outSR = pm_.makeSubRunPrincipal(newSRID.run(),
                                        newSRID.subRun(),
                                        runstart);     // starting time
        put_product_in_principal(std::auto_ptr<double>(new double(7.0)),
                                 *outSR,
                                 "s1");
        readSomething = true;
      }
    if ((*current_)[2] != -1)
      {
        assert(outSR || inSR);
        Timestamp runstart; // current time?
        outE = pm_.makeEventPrincipal(outR?outR->run():inR->run(),
                                      outSR?outSR->subRun():inSR->subRun(),
                                      (*current_)[2],  // event number
                                      runstart);     // starting time
        put_product_in_principal(std::auto_ptr<int>(new int(26)),
                                 *outE,
                                 "m1");
        put_product_in_principal(std::auto_ptr<bool>(new bool(false)),
                                 *outE,
                                 "m2",
                                 "a");
        put_product_in_principal(std::auto_ptr<bool>(new bool(true)),
                                 *outE,
                                 "m2",
                                 "b");
         readSomething = true;
      }
    if (readSomething)
      {
        if (data_.get<bool>("returnFalseWithNewFault", false))
          return false;
      }
    else
       {
         throw Exception(errors::DataCorruption)
           << "Did not read expected info from 'file.'\n";
       }
    ++current_;
    return true;
  }

  void ToyFile::readFile(std::string const &name,
                         art::FileBlock*& fb)
  {
    if (throw_on_readFile_) throw_exception_from("readFile");
    if (!data_.get_if_present(name, fileData_))
      {
        throw art::Exception(art::errors::Configuration)
          << "ToyFile expects to find a parameter representing a file's\n"
          << "contents whose name is "
          << name
          << "\n";
      }
    currentFilename_ = name;
    current_ = fileData_.begin();
    end_ = fileData_.end();
    fb = new art::FileBlock(FileFormatVersion(1, "ToyFile 2011a"),
                            currentFilename_);
  }

  //
  // ToyRawInput is an instantiation of the ReaderSource template.
  //

  typedef art::ReaderSource<ToyFile> ToyRawInput;
}

DEFINE_ART_INPUT_SOURCE(arttest::ToyRawInput)
