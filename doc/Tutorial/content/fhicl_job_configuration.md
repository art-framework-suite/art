---
title: FHiCL Job Configuration
prev: /services
next: /fhicl_sample_include
---

FHiCL Job Configuration
=======================

        #include "job/geometry.fcl"
        #include "job/cosmicgen.fcl"

        process_name: CosmicsGen

        services: {
          TFileService: {
            fileName: "cosmics_hist.root"
          }
          user: @local::ndos_services
        }

        source: {
          module_type: EmptyEvent
          maxEvents:  10 # Number of events to create
        }

        outputs: {
          out1: {
            module_type: RootOutput
            fileName: "cosmics_gen.root"
          }
        }

        physics: {
          producers: {
            generator: @local::cosmics_ndos
            geantgen:  @local::standard_geant4
            photrans:  @local::standard_photrans
            daq:       @local::standard_rsim

	  } # things will be added here

          simulate: [ generator, geantgen, photrans, daq ]
          stream1:  [ out1 ]
          trigger_paths: [simulate]
          end_paths:     [stream1]
        }

        # Existing elements can be replaced ...
        services.user.Geometry:         @local::fd_geo
        services.TFileService.fileName: "mccheckout.root"

        outputs.out1.fileName: "sim_cosmics.root"
