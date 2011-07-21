---
title: Services
prev: /analyzer_interface
next: /fhicl_job_configuration
---

Services
========

* *TFile*: Controls the ROOT directories (one per module) and manages the histogram file.
* *Timing*: Tracks CPU and wall clock time for each module for each event
* *Memory*: Tracks increases in overall program memory on each module invocation
* *FloatingPointControl*: Allows configuration of FPU hardware “exception” processing
* (*RandomNumberService*): Manages the state of a random number stream for each interested module
* (*MessageFacility*): Routes user-emitted messages from modules based on type and severity to destinations

Access interface
----------------

        #include "art/Framework/Services/Optional/TFileService.h"
        ...
        art::ServiceHandle<art::TFileService> tfs;
        fFinalVtxX = tfs->make<TH1F>("fFinalVtxX",
                                     "Circe Vertex X; Xfit-Xmc (cm); Events",
                                     200, -50.0, 50.0);

FHiCL configuration of services
-------------------------------

        services:
        {
          TFileService:
          {
            fileName: "tfile_output.root"
          }

          user:
          {
            # experiment- or user-defined plugin service
          }
          ...
        }
