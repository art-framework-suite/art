---
title: Key Concepts
---

Key concepts of the art framework
=================================


Data-related
------------

* *Event Data Model*: Representation of the data that an experiment collects, all derived information, 
  and historical records necessary for reproduction of results.

* *Data Products*: Experiment defined classes that represent detector signals, reconstruction results,
  simulation, etc..

* *Event*: A collection of data products associated with one time window, the smallest
   unit of detector data collection to be processed.

* *SubRun*: A period of data collection in which operating conditions do not change.

* *Run*: A period of data collection, defined by the experiment.

* *Provenance*: Metadata describing how data products were produced.

Processing-related
------------------

* *Configuration*: Structured documents describing the all processing aspects of a single job
  including setting of parameters and workflow specification.

* *Module*: An object (in C++) that “plugs” into a processing stream and performs a specific task on
  units of data obtained using the Event Data Model, independent of other running modules.

* *Event Processor*: An object that utilizes a configuration document to dynamically schedule sequences
  of modules to process data using the event data model.

* *Services*: A global facility that is dependence on the state of the Event Processor and that can
  be accessed from any module.

next: [The EDM][next]

[next]: /edm