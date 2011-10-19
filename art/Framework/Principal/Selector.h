#ifndef art_Framework_Principal_Selector_h
#define art_Framework_Principal_Selector_h

/*----------------------------------------------------------------------

Classes for all "selector" objects, used to select
EDProducts based on information in the associated Provenance.

Developers who make their own Selector class should inherit
from SelectorBase.

Users can use the classes defined below

  ModuleLabelSelector
  ProcessNameSelector
  ProductInstanceNameSelector

Users can also use the class Selector, which can be constructed given a
logical expression formed from any other selectors, combined with &&
(the AND operator), || (the OR operator) or ! (the NOT operator).

For example, to select only products produced by a module with label
"mymodule" and made in the process "PROD", one can use:

  Selector s( ModuleLabelSelector("mymodule") &&
              ProcessNameSelector("PROD") );

If a module (EDProducter, EDFilter, EDAnalyzer, or OutputModule) is
to use such a selector, it is best to initialize it directly upon
construction of the module, rather than creating a new Selector instance
for every event.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/SelectorBase.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/type_traits"

#include <string>

namespace art {

  template <class A, class B>
  typename
  std::enable_if<std::is_base_of<art::SelectorBase, A>::value && std::is_base_of<art::SelectorBase, B>::value,
                 art::AndHelper<A,B> >::type
  operator&& (A const& a, B const& b);

  template <class A, class B>
  typename
  std::enable_if<std::is_base_of<art::SelectorBase, A>::value && std::is_base_of<art::SelectorBase, B>::value,
                 art::OrHelper<A,B> >::type
  operator|| (A const& a, B const& b);

  template <class A>
  typename
  std::enable_if<std::is_base_of<art::SelectorBase, A>::value,
                 art::NotHelper<A> >::type
  operator! (A const& a);
}

//------------------------------------------------------------------
//
/// Class ProcessNameSelector.
/// Selects EDProducts based upon process name.
///
/// As a special case, a ProcessNameSelector created with the
/// string "*" matches *any* process (and so is rather like having
/// no ProcessNameSelector at all).
//------------------------------------------------------------------

class art::ProcessNameSelector : public art::SelectorBase {
public:
  ProcessNameSelector(const std::string& pn) :
    pn_(pn.empty() ? std::string("*") : pn)
  { }

  virtual ProcessNameSelector* clone() const
  {
    return new ProcessNameSelector(*this);
  }

  std::string const& name() const
  {
    return pn_;
  }

private:
  virtual bool doMatch(BranchDescription const& p) const
  {
    return (pn_=="*") || (p.processName() == pn_);
  }

  std::string pn_;
};

//------------------------------------------------------------------
//
/// Class ProductInstanceNameSelector.
/// Selects EDProducts based upon product instance name.
//
//------------------------------------------------------------------

class art::ProductInstanceNameSelector : public art::SelectorBase {
public:
  ProductInstanceNameSelector(const std::string& pin) :
    pin_(pin)
  { }

  virtual ProductInstanceNameSelector* clone() const
  {
    return new ProductInstanceNameSelector(*this);
  }

private:
  virtual bool doMatch(BranchDescription const& p) const
  {
    return p.productInstanceName() == pin_;
  }

  std::string pin_;
};

//------------------------------------------------------------------
//
/// Class ModuleLabelSelector.
/// Selects EDProducts based upon module label.
//
//------------------------------------------------------------------

class art::ModuleLabelSelector : public art::SelectorBase {
public:
  ModuleLabelSelector(const std::string& label) :
    label_(label)
  { }

  virtual ModuleLabelSelector* clone() const
  {
    return new ModuleLabelSelector(*this);
  }

private:
  virtual bool doMatch(BranchDescription const& p) const
  {
    return p.moduleLabel() == label_;
  }

  std::string label_;
};

//------------------------------------------------------------------
//
/// Class MatchAllSelector.
/// Dummy selector whose match function always returns true.
//
//------------------------------------------------------------------

class art::MatchAllSelector : public art::SelectorBase {
public:
  MatchAllSelector()
  { }

  virtual MatchAllSelector* clone() const
  {
    return new MatchAllSelector;
  }

private:
  virtual bool doMatch(BranchDescription const&) const
  {
    return true;
  }

};

//----------------------------------------------------------
//
// AndHelper template.
// Used to form expressions involving && between other selectors.
//
//----------------------------------------------------------

template <class A, class B>
class art::AndHelper : public SelectorBase {
public:
  AndHelper(A const& a, B const& b) : a_(a), b_(b) { }
  virtual AndHelper *clone() const { return new AndHelper(*this); }
private:
  virtual bool doMatch(BranchDescription const& p) const { return a_.match(p) && b_.match(p); }

  A a_;
  B b_;
};

template <class A, class B>
typename
std::enable_if<std::is_base_of<art::SelectorBase, A>::value &&
               std::is_base_of<art::SelectorBase, B>::value,
               art::AndHelper<A,B> >::type
art::operator&& (A const& a, B const& b) {
  return art::AndHelper<A,B>(a,b);
}

//----------------------------------------------------------
//
// OrHelper template.
// Used to form expressions involving || between other selectors.
//
//----------------------------------------------------------

template <class A, class B>
class art::OrHelper : public art::SelectorBase {
public:
  OrHelper(A const& a, B const& b) : a_(a), b_(b) { }
  virtual OrHelper *clone() { return new OrHelper(*this); }
private:
  virtual bool doMatch(BranchDescription const& p) const { return a_.match(p) || b_.match(p); }

  A a_;
  B b_;
};

template <class A, class B>
typename
std::enable_if<std::is_base_of<art::SelectorBase, A>::value &&
               std::is_base_of<art::SelectorBase, B>::value,
               art::OrHelper<A,B> >::type
art::operator|| (A const& a, B const& b) {
  return art::OrHelper<A,B>(a,b);
}

//----------------------------------------------------------
//
// NotHelper template.
// Used to form expressions involving ! acting on a selector.
//
//----------------------------------------------------------

template <class A>
class art::NotHelper : public art::SelectorBase {
public:
  explicit NotHelper(A const& a) : a_(a) { }
  NotHelper *clone() const { return new NotHelper(*this); }
private:
  virtual bool doMatch(BranchDescription const& p) const { return ! a_.match(p); }

  A a_;
};

template <class A>
typename
std::enable_if<std::is_base_of<art::SelectorBase, A>::value,
               art::NotHelper<A> >::type
art::operator! (A const& a) {
  return art::NotHelper<A>(a);
}

//----------------------------------------------------------
//
// ComposedSelectorWrapper template
// Used to hold an expression formed from the various helpers.
//
//----------------------------------------------------------

template <class T>
class art::ComposedSelectorWrapper : public art::SelectorBase {
public:
  typedef T wrapped_type;
  explicit ComposedSelectorWrapper(T const& t) : expression_(t) { }
  ~ComposedSelectorWrapper() {};
  ComposedSelectorWrapper<T>* clone() const { return new ComposedSelectorWrapper<T>(*this); }
private:
  virtual bool doMatch(BranchDescription const& p) const { return expression_.match(p); }

  wrapped_type expression_;
};

//----------------------------------------------------------
//
// Selector
//
//----------------------------------------------------------

class art::Selector : public art::SelectorBase {
public:
  template <class T> Selector(T const& expression);
  Selector(Selector const& other);
  Selector& operator= (Selector const& other);
  void swap(Selector& other);
  virtual ~Selector();
  virtual Selector* clone() const;

private:
  virtual bool doMatch(BranchDescription const& p) const;

  cet::value_ptr<SelectorBase> sel_;
};

template <class T>
art::Selector::Selector(T const& expression) :
  sel_(new ComposedSelectorWrapper<T>(expression))
{ }

#endif /* art_Framework_Principal_Selector_h */

// Local Variables:
// mode: c++
// End:
