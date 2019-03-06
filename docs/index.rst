.. art documentation master file, created by
   sphinx-quickstart on Sun Jul  8 22:29:32 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

*art*
=====

.. toctree::
   :maxdepth: 2

   Release notes<releaseNotes>

   Depends on<depends>


General *art* and *art* suite information
-----------------------------------------

* `The art Documentation <https://art.fnal.gov/>`_ (This is the art WordPress web site)
* Mailing lists: art-users@fnal.gov (community, also monitored by experts) and artists@fnal.gov (experts).
* `The August, 2015 art/LArSoft course materials <https://indico.fnal.gov/event/9928/>`_
* `Supported Platforms <https://cdcvs.fnal.gov/redmine/projects/cet-is/wiki/Supported_platforms>`_
* `art visual identity <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Art_visual_identity>`_



*art* suite programs
-------------------


Each of the programs below is available if you have setup the art product (either directly, or by having setup you experiment software that uses art).
Each program accepts the flag --help, which prints out instructions for use.

* `art <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Art>`_: this is the event-processing framework executable. Note the availability of --debug-config <file> and --config-out <file> to print out the configuration that you have just told art to use. Note also --annotate, which adds comments that say where each parameter value came from. This command takes into account command-line parameters specified to the art executable, which can insert or modify parameters. The lar and various experiment-specific framework executables all take these same arguments. There is some documentation on the exit status codes that may be returned after an execution of art.
* **art_ut**: this is the event-processing framework executable with built-in support for unit testing using the Boost library. It is intended for writing modules that are used for testing, not for production.
* **fhicl-dump**: this program will read a FHiCL file, fully process it, and print out the FHiCL that describes the resulting ParameterSet. Note that it requires the environment variable FHICL_FILE_PATH to be set. Note that fhicl-dump does not take the large set of flags that the framework executable respects.
* **fhicl-expand**: this program will read a FHiCL file and perform all #include processing, printing the result as a single FHiCL document.
* `cetskelgen <https://cdcvs.fnal.gov/redmine/projects/cetlib/wiki/Cetskelgen>`_. This modular script will generate skeleton source files for the selected module or plugin type. If an experiment designs their own plugin, they can produce a plugin for cetskelgen to generate a skeleton for same.


As of art 3.02, the following programs are provided by the art_root_io package.

* **config_dumper**: this program will read an art/ROOT output file and print out configuration information for the process(es) that created that file.
* **file_info_dumper**: this program will read an art/ROOT output file and has the ability to print the list of events in the file, print the range of events, subruns, and runs that contributed to making the file, and provides access to the internal SQLite database, which can be saved to an external database.
* **count_events**: this program will read an art/ROOT output file and print out how many events are contained in that file.
* `product_sizes_dumper <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Product_sizes_dumper>`_: this program will read and art/ROOT output file and print out information about the sizes of products.
* `sam_metadata_dumper <https://cdcvs.fnal.gov/redmine/projects/art/wiki/SAM_metadata_facilities#sam_metadata_dumper>`_: The sam_metadata_dumper application will read an art-ROOT format file, and extract the information for possible post-processing and upload to SAM.



*art* job configuration
----------------------

* Configuration language and parameters

  * `art framework parameters <https://cdcvs.fnal.gov/redmine/projects/art/wiki/ART_framework_parameters>`_
  * `FHiCL 3 Quick Start Guide <https://cdcvs.fnal.gov/redmine/documents/327>`_

    * `Good art workflow: a presentation <https://indico.fnal.gov/event/9928/session/6/material/0/7>`_

* C++ retrieval of parameters

  * The `fhicl-cpp <https://cdcvs.fnal.gov/redmine/projects/fhicl-cpp/wiki/Wiki>`_ library

* Configuration validation and description

  * `Guidelines for use within art <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Configuration_validation_and_description>`_
  * `The fhicl-cpp validation and description system <https://cdcvs.fnal.gov/redmine/projects/fhicl-cpp/wiki/Configuration_validation_and_fhiclcpp_types>`_



How to use the modularity of *art*
-----------------------------------

* `General design considerations <https://cdcvs.fnal.gov/redmine/projects/art/wiki/General_design_considerations>`_ (please read this first)

* Modules and sources

  * `art Module Design Guide <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Art_Module_Design_Guide>`_
  * `Product Mixing <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Product_Mixing>`_
  * The `ProvenanceDumper <https://cdcvs.fnal.gov/redmine/projects/art/wiki/ProvenanceDumper>`_ output module template
  * `Writing your own input sources <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Writing_your_own_input_sources>`_ using the Source input source template
  * `Sampling input source <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Sampling_input_source>`_ (as of `art 2.12.00 <https://cdcvs.fnal.gov/redmine/versions/1412>`_)

* Tools, services, and other plugins

  * `Guide to writing and using tools <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Guide_to_writing_and_using_tools>`_ (`art 2.06.00 <https://cdcvs.fnal.gov/redmine/versions/1098>`_ and newer)
  * `Guide to writing and using services <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Guide_to_writing_and_using_services>`_
  * `FileCatalogMetadataPlugin: a different type of plugin <https://cdcvs.fnal.gov/redmine/projects/art/wiki/FileCatalogMetadataPlugin>`_
  * `Services that produce data products <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Services_that_produce_data_products>`_ (as of `art 2.10.00 <https://cdcvs.fnal.gov/redmine/versions/1412>`_)



Event processing
----------------

* `Paths <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Paths>`_
* `Filtering events <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Filtering_events>`_


Multithreaded processing (as of art 3)
--------------------------------------

* `Basics <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Art_3_basics>`_
* `Schedules and transitions <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Schedules_and_transitions>`_
* `Module threading types <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Module_threading_types>`_
* `Processing frame <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Processing_frame>`_
* `Parallelism in user code <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Parallelism_in_user_code>`_
* `Upgrading to art 3 <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Upgrading_to_art_3>`_



I/O handling
------------

Output-file handling
~~~~~~~~~~~~~~~~~~~~

* `art/ROOT output file handling <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Output-file_handling>`_ (`art 2.02.00 <https://cdcvs.fnal.gov/redmine/versions/1036>`_ and newer, `art 2.01.00 <https://cdcvs.fnal.gov/redmine/versions/388>`)
* `One output file per input file <https://cdcvs.fnal.gov/redmine/projects/art/wiki/One_output_file_per_input_file>`_ (pre `art 2.01.00 <https://cdcvs.fnal.gov/redmine/versions/388>`_)
* `Output file renaming for ROOT files <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Output_file_renaming_for_ROOT_files>`_

Data products and ROOT dictionaries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* `Data Product Design Guide <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Data_Product_Design_Guide>`_
* `Data Product Dictionary How-To <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Data_Product_Dictionary_How-To>`_
* `Migrating from ROOT5 to ROOT6 <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Migrating_from_ROOT5_to_ROOT6>`_
* `Specifying ROOT compression for data products <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Specifying_ROOT_compression_for_data_products>`_
* `Facilitating Schema Evolution for Data Products <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Facilitating_Schema_Evolution_for_Data_Products>`_



Other information and guides for art suite features and packages
----------------------------------------------------------------

* `Range of validity <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Range_of_validity>`_
* `Concatenating art ROOT files <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Concatenating_art_ROOT_files>`_
* `Consistency of event data products <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Concatenating_art_ROOT_files>`_
* `Customizing messageFacility behavior <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Customizing_messageFacility_behavior>`_
* `Signal handling in art <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Signal_handling_in_art>`_
* `art-provided services <https://cdcvs.fnal.gov/redmine/projects/art/wiki/ART_available_services>`_
* `SAM metadata facilities <https://cdcvs.fnal.gov/redmine/projects/art/wiki/SAM_metadata_facilities>`_
* `SQLite help <https://cdcvs.fnal.gov/redmine/projects/art/wiki/SQLite_help>`_
* `The cetlib utility library <https://cdcvs.fnal.gov/redmine/projects/cetlib/wiki/Wiki>`_
*  Repository browser / LXR for art packages:

   * cetlib

     * `Browser <https://cdcvs.fnal.gov/redmine/projects/cetlib/repository/>`_
     * `LXR <http://cdcvs.fnal.gov/lxr/cetlib/>`_

   * fhicl-cpp 

     * `Browser <https://cdcvs.fnal.gov/redmine/projects/fhicl-cpp/repository/>`_
     * `LXR <http://cdcvs.fnal.gov/lxr/fhicl-cpp/>`_

   * messagefacility

     * `Browser <https://cdcvs.fnal.gov/redmine/projects/messagefacility/repository/>`_
     * `LXR <http://cdcvs.fnal.gov/lxr/messagefacility/>`_

   * art

     * `Browser <https://cdcvs.fnal.gov/redmine/projects/art/repository/>`_
     * `LXR <http://cdcvs.fnal.gov/lxr/art/>`_


General programming information and advice
------------------------------------------

* `Guidelines for the use of Pointers. <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Guidelines_for_the_use_of_Pointers>`_
*  On the use of `asserts vs exceptions. <https://cdcvs.fnal.gov/redmine/projects/art/wiki/AssertsVsExceptions>`_
* `Getting started with ARM (formerly Allinea) Forge tools <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Getting_started_with_MAP_and_DDT>`_ (MAP and DDT). 
* `Getting started with valgrind. <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Getting_started_with_valgrind>`_


Design documentation
--------------------

* `Requirements and design discussion hub <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Requirements_and_design_discussion_hub>`_




































