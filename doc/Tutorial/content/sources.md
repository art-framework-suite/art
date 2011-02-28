---
title: Sources
prev: /modules
next: /filter_interface
---

Sources
=======

* **Source** - provide the stream of Events (e.g. from file)

* New ones be provided by experiment without new **art** release

* Different classes for different tasks:

  * *RootInput*: reads **art** format Root files
  * *NOvARawInputSource*: experiment-specfic source that reads binary DAQ files (provided by framework group)
  * *EmpyEvent*: creates *Events* containing no products (used, e.g., in simulation)