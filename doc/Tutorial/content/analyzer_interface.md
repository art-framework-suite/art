---
title: Analyzer interface
prev: /filter_interface
next: /services
---


Analyzer Interface
==================

    class EDAnalyzer {
      // explicit EDAnalyzer(ParameterSet const&)

      virtual void analyze(Event const&) = 0
      virtual void reconfigure(ParameterSet const&)

      virtual void beginJob()
      virtual void endJob()
      virtual bool beginRun(Run const &)
      virtual bool endRun(Run const &)
      virtual bool beginSubRun(SubRun const &)
      virtual bool endSubRun(SubRun const &)

      virtual void respondToOpenInputFile(FileBlock const& fb)
      virtual void respondToCloseInputFile(FileBlock const& fb)
      virtual void respondToOpenOutputFiles(FileBlock const& fb)
      virtual void respondToCloseOutputFiles(FileBlock const& fb)
    }

