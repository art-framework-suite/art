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
        }
        services.user.Geometry: @local::ndos_geo

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
          producers: { } # things will be added here

          simulate: [ generator, geantgen, photrans, daq ] 
          stream1:  [ out1 ] 
          trigger_paths: [simulate]
          end_paths:     [stream1]
        }
        # New modules can be added as follows...
        physics.producers.geantgen:  @local::std_geant4
        physics.producers.photrans:  @local::std_photrans
        physics.producers.daq:       @local::std_rsim
        physics.producers.generator: @local::cosmics_fd
        
        # Existing elements can be replaced ...
        services.user.Geometry:         @local::fd_geo
        services.TFileService.fileName: "mccheckout.root"
        
        outputs.out1.fileName: "sim_cosmics.root"