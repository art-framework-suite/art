---
title: nova Command Line Arguments
prev: /output_filtering
next: /event_product_access
---

nova Command Line Arguments
============================

* Everything has reasonable default
* Values on the command line override values in the configuration document

        $> nova -h
        nova <options> [config-file]:
          -T [ --TFileName ] arg   File name for TFileService.
          -c [ --config ] arg      Configuration file.
          -e [ --estart ] arg      Event # of first event to process.
          -h [ --help ]            produce help message
          -n [ --nevts ] arg       Number of events to process.
          --nskip arg              Number of events to skip.
          -o [ --output ] arg      Event output stream file.
          -s [ --source ] arg      Source data file (multiple OK).
          -S [ --source-list ] arg file containing a list of source files to read, one
                                   per line.
          --trace                  Activate tracing.
          --notrace                Deactivate tracing.
          --memcheck               Activate monitoring of memory use.
          --nomemcheck             Deactivate monitoring of memory use.
