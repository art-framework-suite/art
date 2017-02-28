# CMSSW interface choices with multithreading

_Below is an assessment based on looking through CMSSW code over the last several days.  It is based on an incomplete understanding.  Information was culled from [this website](https://twiki.cern.ch/twiki/bin/view/CMSPublic/FWMultithreadedFrameworkModuleTypes), and the links therein._

## Introduction

As of October 2016, CMSSW supports:

- serial reading of the input source,
- serial processing of `Run`s and `LuminosityBlock`s, and
- (potentially) parallel processing of events.

<span style="color:red">There is no "opt-in" for thread-safe services--i.e. all services **must be
thread-safe** in the context of CMSSW.</span>

In contrast to a _thread_, the concept of the _stream_ is emphasized
in CMSSW, where each stream has its own event loop, processing events
concurrently with other streams when possible.  There are four
<i>flavor</i>s of modules:

_classic_
  ~ Legacy modules that serialize all processing.

_stream_
  ~ Modules that have one copy per stream.  Member data of user's class therefore does not need to be threadsafe _per se_.

_global_
  ~ Modules that have one instance across all streams.  Member data of user's class **must be** thread-safe.  This is the type of module that yields memory savings.

_one_
  ~ Modules that must serialize some (if not all) processing.  The "classic" module can be equivalently obtained from a specific instantiation of a `one` module.

The classic module base classes reside in the `edm` namespace.  Any
user-provided modules that want to take advantage of parallel
event-processing must inherit from base classes in the
`edm::{global,stream,one}` namespaces.  In other words:

~~~~{.cpp}
namespace edm {

  // The three "Classic" modules
  class EDProducer;
  class EDFilter;
  class EDAnalyzer;

  namespace stream {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }

  namespace global {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }

  namespace one {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }

}
~~~~

###Other comments

* For many functions, CMSSW provides an additional argument that _art_
  does not have--a `const`{.cpp} reference to an `EventSetup` object.
  I will not remark on this difference later on.

* For enabling concurrent module facilities, there are various changes
  to interface that are introduced.  I do not focus on these specific
  changes in interface, but rather concentrate on the overall
  interface choices that CMSSW has made.  For a full description of
  the interface to the CMSSW modules, and how they are adjusted for a
  given template instantiation, please consult
  [the aforementioned website](https://twiki.cern.ch/twiki/bin/view/CMSPublic/FWMultithreadedFrameworkModuleTypes).

* CMSSW does not support concurrent `Run`s and `LuminosityBlock`s at
  this time--they are working toward this, however.

* CMSSW does not support concurrent construction of modules.


##Interface choices made by CMSSW

----

> **Choice 1**: Module base classes used for concurrent event-processing are placed in a separate namespace than the legacy-module base classes.

This is a clean way of separating old functionality from new
functionality.  However, it is worth noting that in the case of CMSSW,
users are expected to know four namespaces.  For a given producer,
they would have to choose between the following base classes:

~~~~{.cpp}
edm::EDProducer;
edm::stream::EDProducer<T...>;
edm::global::EDProducer<T...>;
edm::one::EDProducer<T...>;
~~~~

I find this proliferation of namespaces to be unfortunate, and potentially confusing.  Without having performed an adequate analysis, I would hope that within art, we could instead do something like:

~~~~{.cpp}
art::EDProducer;
art::parallel::EDProducer<T...>;
~~~~

which, I believe, is more in line with usage in the C++ standard
library.  I do not yet know if this is feasible.

----

> **Choice 2**: Module capabilities or _abilities_ (e.g. `BeginRunProducer`, `RunCache<Counter>`) are extended by providing template arguments in the base-class type of the base clause (e.g.):
>
> ~~~~{.cpp}
> class MyProducer : public edm::stream::EDProducer<BeginRunProducer, RunCache<Counter>> { ... };
> ~~~~

Here, the `BeginRunProducer` argument indicates to the framework that
a product will be created in the begin-run phase of `Run` processing.
The `RunCache<Counter>` template argument indicates that a cached
variable of user-specified type `Counter` should be held by the
current `Run` being processed.

The actual template arguments above are irrelevant.  The salient point
is that having a user specify template arguments according to the
desired functionality is very elegant, exposing interface only when it
is needed.  This is an approach we should consider adopting.  See the
choice 3, however.

----

> **Choice 3a**: Extending module abilities requires introducing primarily `static` member functions, _and_ \
> **Choice 3b**: Cached `Global`, `Run`, and `LuminosityBlock` data are held by the base class, and _not_ the derived class.

Choices 3a and 3b are grouped together because they seem to originate
from the same design aspect.  In several cases (perhaps all), the base
class calls the `static`{.cpp} member function that the user provides.
This introduces the following considerations:

* It appears that the intention is for the base class (i.e. the
  framework, and not the user) to own the variables that are meant to
  be cached a given `Run` or `LuminosityBlock`.  Does this extra
  complication provide more thread safety?  I do not know.

* Providing `static`{.cpp} member functions divorces the given
  functionality from any given instance of the class.  To that end,
  CMSSW can ensure that a given `static`{.cpp} member function is
  called the appropriate number of times (e.g. once for
  `globalEndJob`), irrespective of the module flavor.  However, the
  separation of the functionality from the class instance has the
  potential of subverting the encapsulation that good class design
  typically affords.  Specifically, the object-oriented design of
  framework modules is lost whenever the user is expected to specify
  `static`{.cpp} member functions instead of overrides to
  `virtual`{.cpp} functions.  Because of this, I am wary of adopting
  more `static`{.cpp} member functions than are necessary.

----

> **Choice 4**: The original interface functions for legacy-module base classes:
>
> * `virtual void beginRun(Run&)`{.cpp}
> * `virtual void endRun(Run&)`{.cpp}
> * `virtual void beginLuminosityBlock(LuminosityBlock&)`{.cpp}
> * `virtual void endLuminosityBlock(LuminosityBlock&)`{.cpp}
>
>   were changed to:
>
> * `virtual void beginRun(Run const&)`{.cpp}
> * `virtual void endRun(Run const&)`{.cpp}
> * `virtual void beginLuminosityBlock(LuminosityBlock const&)`{.cpp}
> * `virtual void endLuminosityBlock(LuminosityBlock const&)`{.cpp}
>
> In other words, no `Run` nor `LuminosityBlock` products can be inserted from legacy modules.

It would be a good idea for us to clamp down on the number of places
where users can insert products into the `Run` or `SubRun`
transactional objects.  Even requiring users to specify _where_ they
will insert the product would be a benefit as mentioned in choice 2.

----

> **Choice 5**: CMSSW separates `beginRun` (`Run`
> preparation/initialization) from `beginRunProduce`, where the `Run`
> product is actually placed into the `Run`.  This pattern is adopted
> for `LuminosityBlock`s and the matching `end*` interface functions
> as well.

[This website](https://twiki.cern.ch/twiki/bin/view/CMSPublic/FWMultithreadedFrameworkGlobalModuleInterface#edm_BeginRunProducer)
says: _"Items placed during the end of `Run` transition are only
available during `globalEndRun` calls and are not available to the
standard `Stream` method `streamEndRun`.  The reason for this
restriction is the need to make all `Run` products invariant for the
entire processing of the `Run`."_

That said, the benefit of separating the producing function from the processing function is not obvious to me.

## Details

### Classic modules

The interfaces of classic modules and art modules are very similar:

- `begin/endRun` `virtual`{.cpp} functions
- `begin/endLuminosityBlock` (or `SubRun`) `virtual`{.cpp} functions
- `respondToOpen{Close}InputFile` `virtual`{.cpp} functions

Some differences:

- As mentioned above, `Run`/`LuminosityBlock` hooks receive `const`{.cpp}-qualified transactional objects--i.e. with the classic interface, `Run` and `LuminosityBlock` products cannot be inserted.
- `pre/postFork*` hooks are present in CMSSW, but are apparently unused (or, at least, not widely encouraged to be used).

See below for an equivalent interface:

~~~~{.cpp}
namespace edm {
  struct CommonInterface {
    virtual void beginJob() {}
    virtual void endJob() {}

    // Legacy/classic Run and LuminosityBlock hooks do not allow product insertion.
    virtual void beginRun(Run const&, EventSetup const&) {}
    virtual void endRun  (Run const&, EventSetup const&) {}

    virtual void beginLuminosityBlock(LuminosityBlock const&, EventSetup const&) {}
    virtual void endLuminosityBlock  (LuminosityBlock const&, EventSetup const&) {}

    virtual void respondToOpenInputFile (FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}

    virtual void preForkReleaseResources() {}
    virtual void postForkReacquireResources(unsigned childIndex, unsigned numberOfChildren) {}
  };

  class EDAnalyzer : CommonInterface {
  public:
    virtual void analyze(Event const&, EventSetup const&) = 0;
  };

  class EDFilter : CommonInterface {
  public:
    virtual bool filter(Event&, EventSetup const&) = 0;
  };

  class EDProducer : CommonInterface {
  public:
    virtual void produce(Event&, EventSetup const&) = 0;
  };

}
~~~~

An equivalent representation of the classic modules are the "one"
modules where the `edm::one::SharedResources`, `edm::one::WatchRuns`,
and `edm::one::WatchLuminosityBlocks` abilities have been specified.
The abilities of classic modules cannot be extended.

### Modules for multithreading

Modules designed to be used in a multi-threaded context come in three
flavors: "stream", "global", and "one".  A namespace is dedicated for
each flavor:

~~~~{.cpp}
namespace edm {
  namespace stream {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }

  namespace global {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }

  namespace one {
    template <typename... T> class EDProducer;
    template <typename... T> class EDFilter;
    template <typename... T> class EDAnalyzer;
  }
}
~~~~

In contrast to "classic" or "art" modules, these producers are class
templates, where the user-provided template arguments are used to
extend the functionality of the base class.

The intended behavior of each module flavor is described below.

#### Stream

The common interface is:

~~~~{.cpp}
namespace edm::stream {
  struct CommonInterface {
    virtual void beginStream(StreamID) {}
    virtual void endStream() {}

    virtual void beginRun(Run const&, EventSetup const&) {}
    virtual void endRun  (Run const&, EventSetup const&) {}

    virtual void beginLuminosityBlock(LuminosityBlock const&, EventSetup const&) {}
    virtual void endLuminosityBlock  (LuminosityBlock const&, EventSetup const&) {}

    virtual void respondToOpenInputFile (FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}
  };

  template <typename... T>
  class EDAnalyzer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void analyze(Event const&, EventSetup const&) = 0;
  };

  template <typename... T>
  class EDFilter : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual bool filter(Event&, EventSetup const&) = 0;
  };

  template <typename... T>
  class EDProducer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void produce(Event&, EventSetup const&) = 0;
  };
}
~~~~

The abilities that can be added include:

~~~~{.cpp}
namespace edm {
  struct GlobalCache {...};
  struct RunCache {...};
  struct LuminosityBlockCache {...};

  // The following summary features represent means of aggregating information from multiple streams.
  struct RunSummaryCache {...};
  struct LuminosityBlockSummaryCache {...};

  struct BeginRunProducer {...};
  struct EndRunProducer {...};
  struct BeginLuminosityBlockProducer {...};
  struct EndLuminosityBlockProducer {...};
}
~~~~


Data members of all `*Cache` types are marked `mutable`{.cpp} so that
they can be updated.  Not clear to me if this is due to limitations of
C++.  The base class owns the `Run(Summary)` and
`LuminosityBlock(Summary)` cache data--the data is held by
`std::vector<std::shared_ptr<T>>`{.cpp} objects of length 1.  The base
class of the module also owns the `globalCache` data, but by
`std::unique_ptr<T>`{.cpp}.

Specifying any of these abilities in the form (e.g.) `class MyModule :
public EDAnalyzer<GlobalCache<MyType>>`{.cpp} indicates to the system
that a set of (mostly) `static`{.cpp} functions should be called for
that given module.  Unfortunately, this means that the encapsulation
principles of good class design are not exploited, as already
mentioned above.  Granted, it may not be possible for these cases.

#### Global

The interface for global modules is below:

~~~~{.cpp}
namespace edm::global {
  struct CommonInterface {
    virtual void beginJob() {}
    virtual void endJob() {}
  };

  // Event-level functions are const-qualified
  template <typename... T>
  class EDAnalyzer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void analyze(StreamID, Event const&, EventSetup const&) const = 0;
  };

  template <typename... T>
  class EDFilter : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual bool filter(StreamID, Event&, EventSetup const&) const = 0;
  };

  template <typename... T>
  class EDProducer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void produce(StreamID, Event&, EventSetup const&) const = 0;
  };
}
~~~~

Note that all event-level functions are `const`{.cpp}-qualified, and
they also receive `StreamID` arguments.

The additional features to be added include:

~~~~{.cpp}
namespace edm {
  struct StreamCache {...};
  struct RunCache {...};
  struct LuminosityBlockCache {...};

  // The following summary features represent means of aggregating information from multiple streams.
  struct RunSummaryCache {...};
  struct LuminosityBlockSummaryCache {...};

  struct BeginRunProducer {...};
  struct EndRunProducer {...};
  struct BeginLuminosityBlockProducer {...};
  struct EndLuminosityBlockProducer {...};
}
~~~~

#### One

_One_ modules are used whenever thread-safety cannot be guaranteed for the class.

~~~~{.cpp}
namespace edm::one {
  struct CommonInterface {
    virtual void beginJob() {}
    virtual void endJob() {}
  };

  template <typename... T>
  class EDAnalyzer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void analyze(Event const&, EventSetup const&) = 0;
  };

  template <typename... T>
  class EDFilter : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual bool filter(Event&, EventSetup const&) = 0;
  };

  template <typename... T>
  class EDProducer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void produce(Event&, EventSetup const&) = 0;
  };
}
~~~~

Module abilities that can be specified are:

~~~~{.cpp}
namespace edm {
  namespace one {
    struct SharedResources {...};
    struct WatchRuns {...};
    struct WatchLuminosityBlocks {...};
  }
  struct BeginRunProducer {...};
  struct EndRunProducer {...};
  struct BeginLuminosityBlockProducer {...};
  struct EndLuminosityBlockProducer {...};
}
~~~~


### Other comments

One ability remains that can be specified to the module base class:

~~~~{.cpp}
namespace edm {
  struct WatchInputFiles {...};
}
~~~~

Presumably, this is used to allow users to be notified whenever a new
input file is opened.
