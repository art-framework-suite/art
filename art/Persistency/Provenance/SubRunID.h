#ifndef art_Persistency_Provenance_SubRunID_h
#define art_Persistency_Provenance_SubRunID_h
// -*- C++ -*-
//
// Package:     DataFormats/Provenance
// Class  :     SubRunID
//
/**\class SubRunID SubRunID.h DataFormats/Provenance/interface/SubRunID.h

 Description: Holds run and subRun number.

 Usage:
    <usage>

*/
//
//

// system include files
#include <functional>
#include <iosfwd>
#include "boost/cstdint.hpp"

// user include files
#include "art/Persistency/Provenance/RunID.h"

// forward declarations
namespace art {

   typedef unsigned int SubRunNumber_t;


class SubRunID
{

   public:


      SubRunID() : run_(0), subRun_(0) {}
      explicit SubRunID(boost::uint64_t id);
      SubRunID(RunNumber_t iRun, SubRunNumber_t iSubRun) :
	run_(iRun), subRun_(iSubRun) {}

      //virtual ~SubRunID();

      // ---------- const member functions ---------------------
      RunNumber_t run() const { return run_; }
      SubRunNumber_t subRun() const { return subRun_; }

      boost::uint64_t value() const;

      //moving from one SubRunID to another one
      SubRunID next() const {
         if(subRun_ != maxSubRunNumber()) {
            return SubRunID(run_, subRun_+1);
         }
         return SubRunID(run_+1, 1);
      }
      SubRunID nextRun() const {
         return SubRunID(run_+1, 0);
      }
      SubRunID nextRunFirstSubRun() const {
         return SubRunID(run_+1, 1);
      }
      SubRunID previousRunLastSubRun() const {
         if(run_ > 1) {
            return SubRunID(run_-1, maxSubRunNumber());
         }
         return SubRunID(0,0);
      }

      SubRunID previous() const {
         if(subRun_ > 1) {
            return SubRunID(run_, subRun_-1);
         }
         if(run_ != 0) {
            return SubRunID(run_ -1, maxSubRunNumber());
         }
         return SubRunID(0,0);
      }

      bool operator==(SubRunID const& iRHS) const {
         return iRHS.run_ == run_ && iRHS.subRun_ == subRun_;
      }
      bool operator!=(SubRunID const& iRHS) const {
         return ! (*this == iRHS);
      }

      bool operator<(SubRunID const& iRHS) const {
         return doOp<std::less>(iRHS);
      }
      bool operator<=(SubRunID const& iRHS) const {
         return doOp<std::less_equal>(iRHS);
      }
      bool operator>(SubRunID const& iRHS) const {
         return doOp<std::greater>(iRHS);
      }
      bool operator>=(SubRunID const& iRHS) const {
         return doOp<std::greater_equal>(iRHS);
      }

      // ---------- static functions ---------------------------

      static SubRunNumber_t maxSubRunNumber() {
         return 0xFFFFFFFFU;
      }

      static SubRunID firstValidSubRun() {
         return SubRunID(1, 1);
      }
      // ---------- member functions ---------------------------

   private:
      template<template <typename> class Op>
      bool doOp(SubRunID const& iRHS) const {
         //Run takes presidence for comparisions
         if(run_ == iRHS.run_) {
            Op<SubRunNumber_t> op_e;
            return op_e(subRun_, iRHS.subRun_);
         }
         Op<RunNumber_t> op;
         return op(run_, iRHS.run_) ;
      }
      //SubRunID(SubRunID const&); // stop default

      //SubRunID const& operator=(SubRunID const&); // stop default

      // ---------- member data --------------------------------
      RunNumber_t run_;
      SubRunNumber_t subRun_;
};

std::ostream& operator<<(std::ostream& oStream, SubRunID const& iID);

}
#endif /* art_Persistency_Provenance_SubRunID_h */

// Local Variables:
// mode: c++
// End:
