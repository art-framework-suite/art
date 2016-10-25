# CMSSW interface choices with multithreading

## Classic modules

The interfaces of classic modules and art modules are very similar:

- begin/endRun virtual functions
- begin/endLuminosityBlock (or SubRun) virtual functions
- respondToOpen{Close}InputFile virtual functions

Some differences:

- Event-level pure-virtual functions in CMSSW receive an additional argument corresponding to EventSetup.
- Run/LuminosityBlock hooks receive const-qualified transactional objects--i.e. with the classic interface, Run and LuminosityBlock products cannot be inserted.
- pre/postFork* hooks are present in CMSSW, but are apparently unused (or, at least, not widely encouraged to be used).

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

## Modules for multithreading

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
However, it is worth noting, at this point, that users are now
expected to know four namespaces.  For a given producer, they would
have to choose between the following base classes:

~~~~{.cpp}
edm::EDProducer;
edm::stream::EDProducer<T...>;
edm::global::EDProducer<T...>;
edm::one::EDProducer<T...>;
~~~~

I find this proliferation of namespaces to be unfortunate, and
potentially confusing.  Without having performed an adequate
analysis, I would hope that within art, we can do something like:

~~~~{.cpp}
art::EDProducer;
art::parallel::EDProducer<T...>;
~~~~

I do not yet know if this is feasible.  See the "module abilities
and extensions" section below to see what template arguments are
available.

### Stream

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

### Global

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

### One

~~~~{.cpp}
namespace edm::one {
  struct CommonInterface {
    virtual void beginJob() {}
    virtual void endJob() {}
  };

  // Event-level functions are const-qualified
  template <typename... T>
  class EDAnalyzer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void analyze(Event const&, EventSetup const&) const = 0;
  };

  template <typename... T>
  class EDFilter : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual bool filter(Event&, EventSetup const&) const = 0;
  };

  template <typename... T>
  class EDProducer : public CommonInterface, public AdditionalFeature<T>... {
  public:
    virtual void produce(Event&, EventSetup const&) const = 0;
  };
}
~~~~

### Module abilities and extensions

~~~~{.cpp}
namespace edm {
  struct GlobalCache {...};

  struct StreamCache {...};
  struct RunCache {...};
  struct LuminosityBlockCache {...};

  struct RunSummaryCache {...};
  struct LuminosityBlockSummaryCache {...};

  struct BeginRunProducer {...};
  struct EndRunProducer {...};
  struct BeginLuminosityBlockProducer {...};
  struct EndLuminosityBlockProducer {...};

  struct WatchInputFiles {...};
}
~~~~

~~~~{.cpp}
namespace edm::one {
  struct SharedResources {...};
  struct WatchRuns {...};
  struct WatchLuminosityBlocks {...};
}
~~~~

### Module extensions
