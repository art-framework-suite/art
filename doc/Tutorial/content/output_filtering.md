---
title: Output Filtering
prev: /fhicl_sample_include
next: /nova_command_line_args
---

Output Filtering
================

* Any output module can be configured to write out only those events passing a given *trigger path*.
* The parameter set that configures the output module uses a parameter _SelectEvents_ to control the output,
  as shown in the example below

      # this is only a fragment of a full configuration ...
      physics:
      {
        pathA: [ ... ]  # producers and filters are put in this path
        pathB: [ ... ]  # other producers, other filters are put in this path

        outpath: [ pathAwriter ] # output modules and analyzers are put in this path

        trigger_paths: [ pathA, pathB ] # declare that these are "trigger paths"
        end_paths: [ outpath ]          # declare this is an "end path"
      }

      outputs:
      {
        pathAwriter:
        {
          module_type: RootOutput
          fileName: "pathA_events.root"
          SelectEvents: { SelectEvents: [ pathA ] } # Only events passing pathA will be written
        }
      }
