---
title: Filter interface
prev: /sources
next: /analyzer_interface
---

Filter Interface
================

    class EDFilter {
      // explicit EDFilter(ParameterSet const&) 

      virtual bool filter(Event&) = 0
      virtual void reconfigure(ParameterSet const&)

      virtual void beginJob()
      virtual void endJob()
      virtual bool beginRun(Run &)
      virtual bool endRun(Run &)
      virtual bool beginSubRun(SubRun &)
      virtual bool endSubRun(SubRun &)

      virtual void respondToOpenInputFile(FileBlock const& fb)
      virtual void respondToCloseInputFile(FileBlock const& fb)
      virtual void respondToOpenOutputFiles(FileBlock const& fb)
      virtual void respondToCloseOutputFiles(FileBlock const& fb)
    }