---
title: "Configuration: Sample of FHiCL include file"
prev: /fhicl_job_configuration
next: /output_filtering
---

Configuration: Sample of FHiCL include file
===========================================

        BEGIN_PROLOG
        std_cosmictrack:
        {
         module_type:     CosmicTrack
         Debug:           false
         ClusterInput:    "slicer"     # input module of time slices
         DHitGood:        10.          # Maximum distance from the fit
                                       # line for a hit to be considered
                                       # part of the track.  In cm, 
                                       # roughly 1.5 cells.
         ZGapAllowed:     10           # Maximum distance from the fit
                                       # line for a hit to be considered
                                       # part of the track, in planes.
         Ticks:           64000        # number of clock ticks to read out
                                       # one "spill"
         TickWindow:      64           # Size of window in which to look
                                       #for hits to call something an event
         MinHitsInWindow: 10           # Minimum number of hits needed in 
                                       # a window to call something an event
         MinSig:          22.          # Minimum signal for a CellHit::PECorr()
                                       # to have to be used in a track
        }
        END_PROLOG
