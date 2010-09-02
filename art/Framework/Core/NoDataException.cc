#include <art/Framework/Core/NoDataException.h>

namespace edm {
  namespace eventsetup {

NoDataExceptionBase::NoDataExceptionBase(
                      const DataKey& iDataKey,
                      const char* category_name) :
cms::Exception(category_name),
dataKey_(iDataKey),
dataTypeMessage_()
{
}

NoDataExceptionBase::NoDataExceptionBase(
                      const DataKey& iDataKey,
                      const char* category_name ,
                      const std::string& iExtraInfo ) :
cms::Exception(category_name),
dataKey_(iDataKey),
dataTypeMessage_()
{
}

NoDataExceptionBase::~NoDataExceptionBase() throw() {}

const DataKey& NoDataExceptionBase::dataKey() const { return dataKey_; }

std::string NoDataExceptionBase::standardMessage() {
       return std::string(" A provider for this data exists, but it's unable to deliver the data for this \"")
       +"\" record.\n Perhaps no valid data exists for this event? Please check the data's interval of validity.\n";
    }
const std::string &NoDataExceptionBase::beginDataTypeMessage() const
{
  this->dataTypeMessage_ = std::string("No data of type ")
           +"\"";
  return this->dataTypeMessage_;
}
void NoDataExceptionBase::endDataTypeMessage() const
{
   this->dataTypeMessage_ +"\" with label "
           +"\""
           +this->dataKey_.name().value()
           +"\" "
           +"in record \""
           +"\"";
}

  }
}
