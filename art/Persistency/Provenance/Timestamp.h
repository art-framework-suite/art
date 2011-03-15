#ifndef art_Persistency_Provenance_Timestamp_h
#define art_Persistency_Provenance_Timestamp_h
//
// OriginalAuthor:      Chris Jones
// Created:             Thu Mar 24 16:23:05 EST 2005
//

// system include files
#include "cpp0x/cstdint"

// user include files

// forward declarations
namespace art {
   typedef uint64_t TimeValue_t;
   class Timestamp;
}

class art::Timestamp {
public:
   Timestamp(TimeValue_t iValue) :
      timeLow_(static_cast<uint32_t>(lowMask() & iValue)),
      timeHigh_(static_cast<uint32_t>(iValue >> 32))
   {}

   Timestamp() :
      timeLow_(invalidTimestamp().timeLow_),
      timeHigh_(invalidTimestamp().timeHigh_)
   {}

   virtual ~Timestamp() {}

   TimeValue_t value() const {
      return (static_cast<uint64_t>(timeHigh_) << 32) | timeLow_;
   }

   uint32_t timeLow() const { return timeLow_; }

   uint32_t timeHigh() const { return timeHigh_; }

   // ---------- const member functions ---------------------
   bool operator==(const Timestamp& iRHS) const {
      return timeHigh_ == iRHS.timeHigh_ &&
         timeLow_ == iRHS.timeLow_;
   }

   bool operator!=(const Timestamp& iRHS) const {
      return !(*this == iRHS);
   }

   bool operator<(const Timestamp& iRHS) const {
      if(timeHigh_ == iRHS.timeHigh_) {
         return timeLow_ < iRHS.timeLow_;
      }
      return timeHigh_ < iRHS.timeHigh_;
   }

   bool operator<=(const Timestamp& iRHS) const {
      if(timeHigh_ == iRHS.timeHigh_) {
         return timeLow_ <= iRHS.timeLow_;
      }
      return timeHigh_ <= iRHS.timeHigh_;
   }

   bool operator>(const Timestamp& iRHS) const {
      if(timeHigh_ == iRHS.timeHigh_) {
         return timeLow_ > iRHS.timeLow_;
      }
      return timeHigh_ > iRHS.timeHigh_;
   }

   bool operator>=(const Timestamp& iRHS) const {
      if(timeHigh_ == iRHS.timeHigh_) {
         return timeLow_ >= iRHS.timeLow_;
      }
      return timeHigh_ >= iRHS.timeHigh_;
   }

   // ---------- static member functions --------------------
   static const Timestamp& invalidTimestamp() {
      static Timestamp const s_invalid(0);
      return s_invalid;
   }

   static const Timestamp& endOfTime() {
      static Timestamp const s_endOfTime(-1);
      return s_endOfTime;
   }

   static const Timestamp& beginOfTime() {
      static Timestamp const s_beginOfTime(1);
      return s_beginOfTime;
   }

private:
   // ---------- member data --------------------------------
   // ROOT does not support ULL
   //TimeValue_t time_;
   uint32_t timeLow_;
   uint32_t timeHigh_;

   TimeValue_t lowMask() const {
      static TimeValue_t const s_lowMask = 0xFFFFFFFF;
      return s_lowMask;
   }
};
#endif /* art_Persistency_Provenance_Timestamp_h */

// Local Variables:
// mode: c++
// End:
