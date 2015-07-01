#ifndef art_Framework_Core_GroupSelectorRules_h
#define art_Framework_Core_GroupSelectorRules_h

// ======================================================================
//
// GroupSelectorRules: rules to select specific groups in an event.
//
// ======================================================================

#include <string>
#include <vector>

namespace art {
  // defined below:
  class GroupSelectorRules;

  // forward declarations:
  class BranchDescription;
  class GroupSelector;
}

// ----------------------------------------------------------------------

class art::GroupSelectorRules
{
public:
  GroupSelectorRules(std::vector<std::string> const& commands,
                     std::string const& parameterName,
                     std::string const& parameterOwnerName);

  //--------------------------------------------------
  // BranchSelectState associates a BranchDescription
  // (*desc) with a bool indicating whether or not the branch with
  // that name is to be selected.  Note that parameter bd may not be null.
  struct BranchSelectState
  {
    BranchDescription const* desc;
    bool                     selectMe;

    // N.B.: We assume bd is not null.
    explicit BranchSelectState (BranchDescription const* bd) :
      desc    (bd),
      selectMe(false)
    { }
  };  // BranchSelectState

  void applyToAll(std::vector<BranchSelectState>& branchstates) const;

  bool keepAll() const {return keepAll_;}

private:
  class Rule
  {
  public:
    Rule(std::string const& s,
         std::string const& parameterName,
         std::string const& owner);

    // Apply the rule to all the given branch states. This may modify
    // the given branch states.
    void applyToAll(std::vector<BranchSelectState>& branchstates) const;

    // If this rule applies to the given BranchDescription, then
    // modify 'result' to match the rule's select flag. If the rule does
    // not apply, do not modify 'result'.
    void applyToOne(BranchDescription const* branch, bool& result) const;

    // Return the answer to the question: "Does the rule apply to this
    // BranchDescription?"
    bool appliesTo(BranchDescription const* branch) const;

  private:
    // selectflag_ carries the value to which we should set the 'select
    // bit' if this rule matches.
    bool        selectflag_;
    std::string productType_;
    std::string moduleLabel_;
    std::string instanceName_;
    std::string processName_;
  };  // Rule

private:
  std::vector<Rule> rules_;
  bool              keepAll_;
};  // GroupSelectorRules

// ======================================================================

#endif /* art_Framework_Core_GroupSelectorRules_h */

// Local Variables:
// mode: c++
// End:
