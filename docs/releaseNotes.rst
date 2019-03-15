*art* series 3.02
=================



This is the first release that removes the ROOT dependency from core art functionality. Two new packages have been introduced:

* art_root_io: provides ROOT functionality required for users (e.g. RootInput)
* critic: umbrella UPS product that sets up a consistent set of art, art_root_io, and gallery versions

Please consult the list of breaking changes to determine how your code should be adjusted to handle this migration.

.. Optional description of series


New features
------------

Various new features have been added, primarily addressing usability issues:

* The '-e|--estart' program option now accepts a triplet of numbers corresponding to an art::EventID instead of one number (resolves issue #9594)
* FileDumperOutput can now print out the art::ProductID along with the other product information (resolves issue #18153)
* The SAM metadata stored in an art/ROOT file has been adjusted to better match what SAM expects (resolves issue #18983)
* Output-file renaming has been extended to allow for Run and SubRun timestamps in the output file (resolves issue #19374)
* Configuration validation is now supported for MixFilter detail classes (resolves issue #19970)
* MixFilter detail classes can now directly call MixHelper::createEngine to get a reference to the art-managed random-number-engine (resolves issue #20063)
* Other minor features
.. New features

Other
-----

Platform/compiler support
~~~~~~~~~~~~~~~~~~~~~~~~~

This series is the first to support macOS Mojave. As of art 3.02.00, art will no longer support macOS Sierra. 
In addition, the following compilers have been introduced for art 3.02.00:

* GCC 8.2.0 with C++17 enabled (e19 qualifier)
* Clang 7.0.0 with C++17 enabled (c2 qualifier)

See here for more detailed information about primary qualifiers.


Python 3 support
~~~~~~~~~~~~~~~~

This series is the first to support Python 3, allowing users roughly 1 year to switch to Python 3 before the Python 2's end-of-life of January 1, 2020. 
Note that for technical reasons, Python 3 builds are not available for SLF6 platforms. 
In addition, the fhicl Python extension module is only supported for version 3.02.03 and newer.



.. Other

Breaking changes
----------------

Please consult the list of `breaking changes <https://cdcvs.fnal.gov/redmine/projects/art/wiki/302_breaking_changes>`_ to determine if/how your code should be modified.


.. Breaking changes


.. 
    h3(#releases){background:darkorange}. %{color:white}&nbsp; _art_ releases%


*art* suite release notes 3.02.01 (2019/02/07)
==============================================


* An *art 3.02* `release <releaseNotes>`_
* `Download page <https://scisoft.fnal.gov/scisoft/bundles/art/3.02.01/art-3.02.01.html>`_

External package changes
------------------------

* package 1
  
  * vX_YY_ZZ (prev) -> vU_VV_WW (this version)
  * *Build only dependency*

* package 2

  * vX_YY_ZZ (prev) -> vU_VV_WW (this version)
  * *Build only dependency*
.. External package changes

Bug fixes
---------

No bugs fixed in this release
.. Bug fixes





Known issues
------------

An incorrect attempt at re-enabling the fhicl Python extension module resulted in failed builds for Python 3. 
This error has been fixed in art suite `3.02.03 <index.html>`_.


------------

Depends on
----------

* canvas 3.07.01
* cetlib 3.07.01
* cetlib_except 1.03.02
* fhicl-cpp 4.09.01
* hep_concurrency 1.03.02
* messagefacility 2.04.01


..
    ###
    ### The following are lines that should be placed in the release notes
    ### pages of individual packages.
    ###

