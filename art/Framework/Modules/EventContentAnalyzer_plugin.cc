// ======================================================================
//
// EventContentAnalyzer_plugin: print out what data is contained within
//                              an Event at that point in the path
//
// ======================================================================


#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GenericHandle.h"
#include "art/Framework/Core/MakerMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/ParameterSet/ParameterSetDescription.h"
#include "art/Persistency/Provenance/Provenance.h"
#include "art/Utilities/Algorithms.h"

#include "MessageFacility/MessageLogger.h"

#include <exception>
#include <iomanip>  // hex, dec
#include <map>
#include <ostream> // endl
#include <sstream>
#include <string>
#include <vector>


// Contents:
namespace edm {
  class EventContentAnalyzer;
}
using edm::EventContentAnalyzer;


// ======================================================================


class edm::EventContentAnalyzer
  : public EDAnalyzer
{
public:
  explicit EventContentAnalyzer(fhicl::ParameterSet const & );
  ~EventContentAnalyzer();

  virtual void analyze( edm::Event const & );
  virtual void endJob();

  static void fillDescription( edm::ParameterSetDescription &
                             , std::string const & moduleLabel );

private:
  // ----------member data ---------------------------
  std::string                 indentation_;
  std::string                 verboseIndentation_;
  std::vector<std::string>    moduleLabels_;
  bool                        verbose_;
  std::vector<std::string>    getModuleLabels_;
  bool                        getData_;
  int                         evno_;
  std::map<std::string, int>  cumulates_;

};  // EventContentAnalyzer


// ======================================================================


// consistently format class names
static std::string
  formatClassName( std::string const & iName )
{
  return std::string("(") + iName + ")";
}


static const char * kNameValueSep = "=";


// print the object information after conversion to the correct type
template< typename T >
static void
  doPrint( std::string    const & iName
         , Reflex::Object const & iObject
         , std::string    const & iIndent )
{
  mf::LogAbsolute("EventContent")
    << iIndent << iName << kNameValueSep
    << *reinterpret_cast<T*>( iObject.Address() );
}


template<>
void
  doPrint<char>( std::string    const & iName
               , Reflex::Object const & iObject
               , std::string    const & iIndent )
{
  mf::LogAbsolute("EventContent")
    << iIndent << iName << kNameValueSep
    << static_cast<int>( *reinterpret_cast<char*>( iObject.Address() ) );
}


template<>
void
  doPrint<unsigned char>( std::string    const & iName
                        , Reflex::Object const & iObject
                        , std::string    const & iIndent )
{
  typedef  unsigned char  uchar;
  typedef  unsigned int   uint;

  mf::LogAbsolute("EventContent")
    << iIndent << iName << kNameValueSep
    << static_cast<uint>( *reinterpret_cast<uchar*>( iObject.Address() ) );
}

template<>
void
  doPrint<bool>( std::string    const & iName
               , Reflex::Object const & iObject
               , std::string    const & iIndent )
{
  mf::LogAbsolute("EventContent")
    << iIndent << iName << kNameValueSep
    << ( (*reinterpret_cast<bool*>( iObject.Address() )) ? "true"
                                                         : "false" );
}


typedef void (*FunctionType)( std::string    const &
                            , Reflex::Object const &
                            , std::string    const & );

typedef std::map<std::string, FunctionType> TypeToPrintMap;


template<typename T>
static void
  addToMap( TypeToPrintMap & iMap )
{
  iMap[ typeid(T).name() ] = doPrint<T>;
}

static bool printAsBuiltin( std::string    const & iName
                          , Reflex::Object const   iObject
                          , std::string    const & iIndent )
{
  static TypeToPrintMap  s_map;
  static bool isFirst = true;
  if( isFirst ) {
    addToMap<bool          >(s_map);
    addToMap<char          >(s_map);
    addToMap<short         >(s_map);
    addToMap<int           >(s_map);
    addToMap<long          >(s_map);
    addToMap<unsigned char >(s_map);
    addToMap<unsigned short>(s_map);
    addToMap<unsigned int  >(s_map);
    addToMap<unsigned long >(s_map);
    addToMap<float         >(s_map);
    addToMap<double        >(s_map);
    isFirst = false;
  }

  TypeToPrintMap::iterator itFound
    = s_map.find( iObject.TypeOf().TypeInfo().name() );

  if( itFound == s_map.end() )
    return false;

  itFound->second(iName,iObject,iIndent);
  return true;

}  // printAsBuiltin()


static bool
  printAsContainer( std::string    const & iName
                  , Reflex::Object const & iObject
                  , std::string    const & iIndent
                  , std::string    const & iIndentDelta );


static void
  printObject( std::string    const & iName
             , Reflex::Object const & iObject
             , std::string    const & iIndent
             , std::string    const & iIndentDelta )
{
  std::string printName = iName;
  Reflex::Object objectToPrint = iObject;
  std::string indent( iIndent );

  if( iObject.TypeOf().IsPointer() ) {
    mf::LogAbsolute("EventContent")
      << iIndent << iName << kNameValueSep
      << formatClassName(iObject.TypeOf().Name())
      << std::hex << iObject.Address() << std::dec;
      Reflex::Type pointedType = iObject.TypeOf().ToType();
      if(     Reflex::Type::ByName("void") == pointedType
          || pointedType.IsPointer()
          || iObject.Address() == 0 ) {
        return;
      }
    return;

    //have the code that follows print the contents of the data to which the pointer points
    objectToPrint = Reflex::Object(pointedType, iObject.Address());
    //try to convert it to its actual type (assuming the original type was a base class)
    objectToPrint = Reflex::Object(objectToPrint.CastObject(objectToPrint.DynamicType()));
    printName = std::string(" * ")+iName;
    indent +=iIndentDelta;
  }

  std::string typeName( objectToPrint.TypeOf().Name() );
  if( typeName.empty() )
    typeName="<unknown>";

  //see if we are dealing with a typedef
  if( objectToPrint.TypeOf().IsTypedef() )
    objectToPrint = Reflex::Object(objectToPrint.TypeOf().ToType(),objectToPrint.Address());
  if( printAsBuiltin(printName,objectToPrint,indent) )
    return;
  if( printAsContainer(printName,objectToPrint,indent,iIndentDelta) )
    return;

  mf::LogAbsolute("EventContent")
    << indent << printName << " " << formatClassName(typeName);
  indent += iIndentDelta;
  //print all the data members
  for( Reflex::Member_Iterator itMember = objectToPrint.TypeOf().DataMember_Begin()
     ; itMember != objectToPrint.TypeOf().DataMember_End(); ++itMember ) {
    try {
      printObject( itMember->Name()
                 , itMember->Get(objectToPrint)
                 , indent
                 , iIndentDelta );
    }
    catch( std::exception & iEx ) {
      mf::LogAbsolute("EventContent")
        << indent << itMember->Name()
        << " <exception caught(" << iEx.what() << ")>\n";
      }
   }
}  // printObject()


static bool
  printAsContainer( std::string    const & iName
                  , Reflex::Object const & iObject
                  , std::string    const & iIndent
                  , std::string    const & iIndentDelta )
{
  Reflex::Object sizeObj;
  try {
    sizeObj = iObject.Invoke("size");
    assert( sizeObj.TypeOf().TypeInfo() == typeid(size_t) );
    size_t size = *reinterpret_cast<size_t*>( sizeObj.Address() );
    Reflex::Member atMember;
    try {
      atMember = iObject.TypeOf().MemberByName("at");
    }
    catch(const std::exception & x) {
      return false;
    }
    mf::LogAbsolute("EventContent")
      << iIndent << iName << kNameValueSep << "[size=" << size << "]";
    Reflex::Object contained;
    std::string indexIndent = iIndent + iIndentDelta;
    for( size_t index = 0; index != size; ++index ) {
      std::ostringstream sizeS;
      sizeS << "[" << index << "]";
      contained = atMember.Invoke( iObject
                                 , Reflex::Tools::MakeVector(static_cast<void*>( &index ))
                                 );
      try {
        printObject(sizeS.str(),contained,indexIndent,iIndentDelta);
      }
      catch(std::exception & iEx) {
        mf::LogAbsolute("EventContent")
          << indexIndent << iName
          << " <exception caught(" << iEx.what() << ")>\n";
      }
    }
    return true;
  }
  catch( std::exception const & x ) {
    return false;
  }
  return false;
}


static void
  printObject( edm::Event  const & iEvent
             , std::string const & iClassName
             , std::string const & iModuleLabel
             , std::string const & iInstanceLabel
             , std::string const & iProcessName
             , std::string const & iIndent
             , std::string const & iIndentDelta )
{
  using namespace edm;
  try {
    GenericHandle handle(iClassName);
  }
  catch( edm::Exception const & ) {
    mf::LogAbsolute("EventContent")
      << iIndent << " \"" << iClassName << "\""
      << " is an unknown type" << std::endl;
    return;
  }
  GenericHandle handle(iClassName);
  iEvent.getByLabel( edm::InputTag(iModuleLabel,iInstanceLabel,iProcessName)
                   ,handle );
  std::string className = formatClassName(iClassName);
  printObject(className, * handle,iIndent,iIndentDelta);
}


// ======================================================================


// constructors and destructor

EventContentAnalyzer::EventContentAnalyzer( fhicl::ParameterSet const & ps )
: indentation_       ( ps.getUntrackedParameter( "indentation"
                                               , std::string("++")) )
, verboseIndentation_( ps.getUntrackedParameter( "verboseIndentation"
                                               , std::string("  ")) )
, moduleLabels_      ( ps.getUntrackedParameter( "verboseForModuleLabels"
                                               , std::vector<std::string>()) )
, verbose_           ( ps.getUntrackedParameter("verbose",false)
                     || moduleLabels_.size() > 0 )
, getModuleLabels_   ( ps.getUntrackedParameter( "getDataForModuleLabels"
                                               , std::vector<std::string>()) )
, getData_           ( ps.getUntrackedParameter("getData",false)
                     || getModuleLabels_.size() > 0 )
, evno_              ( 1 )
{
  edm::sort_all(moduleLabels_);
}


EventContentAnalyzer::~EventContentAnalyzer()
{ }


// member functions

// ------------ method called to produce the data  ------------
void
  EventContentAnalyzer::analyze( edm::Event const & iEvent )
{
  using namespace edm;

  typedef std::vector< Provenance const * > Provenances;
  Provenances provenances;
  std::string friendlyName;
  std::string modLabel;
  std::string instanceName;
  std::string processName;
  std::string key;

  iEvent.getAllProvenance(provenances);

  mf::LogAbsolute("EventContent")
    << "\n" << indentation_ << "Event " << std::setw(5) << evno_
    << " contains "
    << provenances.size() << " product" << (provenances.size()==1 ?"":"s")
    << " with friendlyClassName, moduleLabel, productInstanceName and processName:"
    << std::endl;

  std::string startIndent = indentation_+verboseIndentation_;
  for( Provenances::iterator itProv = provenances.begin()
                           , itProvEnd = provenances.end()
     ; itProv != itProvEnd; ++itProv) {
    friendlyName = ( * itProv)->friendlyClassName();
    //if(friendlyName.empty())  friendlyName = std::string("||");

    modLabel = ( * itProv)->moduleLabel();
    //if(modLabel.empty())  modLabel = std::string("||");

    instanceName = ( * itProv)->productInstanceName();
    //if(instanceName.empty())  instanceName = std::string("||");

    processName = ( * itProv)->processName();

    mf::LogAbsolute("EventContent")
      << indentation_ << friendlyName
      << " \"" << modLabel
      << "\" \"" << instanceName << "\" \""
      << processName << "\""
      << std::endl;

    key = friendlyName
        + std::string(" + \"") + modLabel
        + std::string("\" + \"") + instanceName + "\" \"" + processName + "\"";
    ++cumulates_[key];

    if( verbose_ ) {
      if(    moduleLabels_.empty()
          || edm::binary_search_all(moduleLabels_, modLabel) ) {
        //indent one level before starting to print
        printObject( iEvent
                   , (*itProv)->className()
                   , (*itProv)->moduleLabel()
                   , (*itProv)->productInstanceName()
                   , (*itProv)->processName()
                   , startIndent
                   , verboseIndentation_ );
        continue;
      }
    }
    if( getData_ ) {
      if(    getModuleLabels_.empty()
          || edm::binary_search_all(getModuleLabels_, modLabel) ) {
        const std::string & className = ( * itProv)->className();
        using namespace edm;
        try {
          GenericHandle handle(className);
        }
        catch( edm::Exception const & ) {
          mf::LogAbsolute("EventContent")
            << startIndent << " \"" << className << "\""
            << " is an unknown type" << std::endl;
          return;
        }
        GenericHandle handle(className);
        iEvent.getByLabel( edm::InputTag((*itProv)->moduleLabel()
                         , (*itProv)->productInstanceName()
                         , (*itProv)->processName())
                         , handle );
      }
    }
  }
  ++evno_;
}  // analyze()


// ------------ method called at end of job -------------------
void
  EventContentAnalyzer::endJob()
{
  typedef std::map<std::string,int> nameMap;

  mf::LogAbsolute("EventContent")
    << "\nSummary for key being the concatenation of friendlyClassName,"
       " moduleLabel, productInstanceName and processName" << std::endl;
  for( nameMap::const_iterator it = cumulates_.begin()
                             , itEnd = cumulates_.end()
     ; it != itEnd; ++it ) {
    mf::LogAbsolute("EventContent")
      << std::setw(6) << it->second << " occurrences of key "
      << it->first << std::endl;
  }
}


void
  EventContentAnalyzer::fillDescription( edm::ParameterSetDescription & iDesc
                                       , std::string const & moduleLabel )
{
  std::string defaultString("++");
  iDesc.addOptionalUntracked<std::string>( "indentation", defaultString);

  defaultString = "  ";
  iDesc.addOptionalUntracked<std::string>( "verboseIndentation", defaultString);

  std::vector<std::string> defaultVString;
  iDesc.addOptionalUntracked<std::vector<std::string> >("verboseForModuleLabels", defaultVString);

  iDesc.addOptionalUntracked<bool>("verbose", false);

  iDesc.addOptionalUntracked<std::vector<std::string> >("getDataForModuleLabels", defaultVString);

  iDesc.addOptionalUntracked<bool>("getData", false);
}


// ======================================================================


DEFINE_FWK_MODULE(EventContentAnalyzer);
