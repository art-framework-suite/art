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

