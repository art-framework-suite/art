#ifndef DataFormats_Common_HLTGlobalStatus_h
#define DataFormats_Common_HLTGlobalStatus_h

/** \class art::HLTGlobalStatus
 *
 *
 *  The HLT global status, summarising the status of the individual
 *  HLT triggers, is implemented as a vector of HLTPathStatus objects.
 *
 *  If the user wants map-like indexing of HLT triggers through their
 *  names as key, s/he must use the TriggerNamesService.
 *
 *
 *
 *
 *  \author Martin Grunewald
 *
 */

#include "art/Persistency/Common/HLTenums.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include <vector>
#include <ostream>

// ----------------------------------------------------------------------

namespace art {

   class HLTGlobalStatus {
   private:
      // Status of each HLT path
      std::vector<HLTPathStatus> paths_;

   public:

      // Constructor - for n paths
      HLTGlobalStatus(const unsigned int n=0) : paths_(n) {}

         // Get number of paths stored
         unsigned int size() const { return paths_.size(); }

         // Reset status for all paths
         void reset() {
            const unsigned int n(size());
            for (unsigned int i = 0; i != n; ++i) paths_[i].reset();
         }

         // global "state" variables calculated on the fly!

         // Was at least one path run?
         bool wasrun() const {return State(0);}
         // Has at least one path accepted the event? If no paths were
         // run, or there are no paths, the answer is, "yes."
         bool accept() const {return State(1);}
         // Has any path encountered an error (exception)
         bool  error() const {return State(2);}

         // get hold of individual elements, using safe indexing with "at" which throws!

         const HLTPathStatus& at (const unsigned int i)   const { return paths_.at(i); }
         HLTPathStatus& at (const unsigned int i)         { return paths_.at(i); }
         const HLTPathStatus& operator[](const unsigned int i) const { return paths_.at(i); }
         HLTPathStatus& operator[](const unsigned int i)       { return paths_.at(i); }

         // Was ith path run?
         bool wasrun(const unsigned int i) const { return at(i).wasrun(); }
         // Has ith path accepted the event?
         bool accept(const unsigned int i) const { return at(i).accept(); }
         // Has ith path encountered an error (exception)?
         bool  error(const unsigned int i) const { return at(i).error() ; }

         // Get status of ith path
         hlt::HLTState state(const unsigned int i) const { return at(i).state(); }
         // Get index (slot position) of module giving the decision of the ith path
         unsigned int  index(const unsigned int i) const { return at(i).index(); }
         // Reset the ith path
         void reset(const unsigned int i) { at(i).reset(); }
         // swap function
         void swap(HLTGlobalStatus& other) { paths_.swap(other.paths_); }
         // copy assignment implemented with swap()
         HLTGlobalStatus& operator=(HLTGlobalStatus const& rhs) {
            HLTGlobalStatus temp(rhs);
            this->swap(temp);
            return *this;
         }

   private:
         // Global state variable calculated on the fly
         bool State(unsigned int icase) const {
            bool flags[3] = {false, false, false};
            const unsigned int n(size());
            for (unsigned int i = 0; i != n; ++i) {
               const hlt::HLTState s(state(i));
               if (s!=hlt::Ready) {
                  flags[0]=true;       // at least one trigger was run
                  if (s==hlt::Pass) {
                     flags[1]=true;     // at least one trigger accepted
                  } else if (s==hlt::Exception) {
                     flags[2]=true;     // at least one trigger with error
                  }
               }
            }
            // Change in semantics of flags[1] vs pre-ART: now we accept if
            // no paths were run.
            flags[1] |= (!flags[0]) | paths_.empty();
            return flags[icase];
         }

   };  // HLTGlobalStatus

   // Free swap function
   inline void
      swap(HLTGlobalStatus& lhs, HLTGlobalStatus& rhs)
      {
         lhs.swap(rhs);
      }

   // Formatted printout of trigger tbale
   inline std::ostream&
      operator <<(std::ostream& ost, const HLTGlobalStatus& hlt)
      {
         std::vector<std::string> text(4); text[0]="n"; text[1]="1"; text[2]="0"; text[3]="e";
         const unsigned int n(hlt.size());
         for (unsigned int i = 0; i != n; ++i) ost << text.at(hlt.state(i));
         return ost;
      }

}  // art

// ----------------------------------------------------------------------

// The standard allows us to specialize std::swap for non-templates.
// This ensures that HLTGlobalStatus::swap() will be used in algorithms.

namespace std {

  template <>
  inline void
    swap(art::HLTGlobalStatus& lhs, art::HLTGlobalStatus& rhs)
  {
    lhs.swap(rhs);
  }

}

// ======================================================================

#endif
