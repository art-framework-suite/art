# ROOT use in the _art_ suite from a threading perspective.

## Overview.

Non-test files including ROOT headers comprise 58 files, split between _canvas_ and _art_:

* `art/art/Framework/Art/InitRootHandlers.cc`

    Basic ROOT setup: error handling and behavior modification: invoked only during single-threaded parts of the _art_ system.

* `art/art/Framework/Modules/MixFilter.h`\
`art/art/Framework/IO/ProductMix/MixHelper.cc`\
`art/art/Framework/IO/ProductMix/MixHelper.h`\
`art/art/Framework/IO/ProductMix/ProdToProdMapBuilder.cc`

    ROOT files are opened as necessary, including during the event loop, but access to a given TFile is confined to a single module instance. Tree access is similarly ubiquitous. The types to be read are made known by user calls to `art::MixHelper::produces(...)`{.cpp} at module construction time, which implies that dictionaries are properly initialized in single-threaded mode. However, provided direct access to the ROOT file is serialized, it should be possible to enable a given instance of an `art::MixFilter<>'{.cpp} instantiation to 

* `art/art/Framework/IO/Root/count_events.cc`\
`art/art/Framework/IO/Root/config_dumper.cc`\
`art/art/Framework/IO/Root/file_info_dumper.cc`\
`art/art/Framework/IO/Root/product_sizes_dumper.cc`\
`art/art/Framework/IO/Root/sam_metadata_dumper.cc`

    These are standalone programs and therefore ROOT use has no threading implications.

* `art/art/Framework/Services/Optional/TFileService_service.cc`\
`art/art/Framework/Services/Optional/TFileDirectory.cc`\
`art/art/Framework/Services/Optional/TFileDirectory.h`\
`art/art/Framework/Services/Optional/detail/TH1AddDirectorySentry.cc`

    These files collectively provide the `art::TFileService`{.cpp} service. See below for more details.

* `art/art/Framework/IO/Root/BranchMapperWithReader.h`\
`art/art/Framework/IO/Root/Inputfwd.h`\
`art/art/Framework/IO/Root/RootBranchInfo.h`\
`art/art/Framework/IO/Root/RootBranchInfoList.cc`\
`art/art/Framework/IO/Root/RootBranchInfoList.h`\
`art/art/Framework/IO/Root/RootDelayedReader.cc`\
`art/art/Framework/IO/Root/RootInputFile.cc`\
`art/art/Framework/IO/Root/RootInputFileSequence.cc`\
`art/art/Framework/IO/Root/RootInput_source.cc`\
`art/art/Framework/IO/Root/RootOutputFile.cc`\
`art/art/Framework/IO/Root/RootOutputFile.h`\
`art/art/Framework/IO/Root/RootOutputTree.cc`\
`art/art/Framework/IO/Root/RootOutputTree.h`\
`art/art/Framework/IO/Root/RootSizeOnDisk.cc`\
`art/art/Framework/IO/Root/RootSizeOnDisk.h`\
`art/art/Framework/IO/Root/RootTree.cc`\
`art/art/Framework/IO/Root/RootTree.h`\
`art/art/Framework/IO/Root/detail/DummyProductCache.cc`\
`art/art/Framework/IO/Root/detail/InfoDumperInputFile.h`\
`art/art/Framework/IO/Root/detail/getEntry.cc`\
`art/art/Framework/IO/Root/detail/resolveRangeSet.h`\
`art/art/Framework/IO/Root/detail/rootFileSizeTools.cc`\
`art/art/Framework/IO/Root/detail/rootFileSizeTools.h`\
`art/art/Framework/IO/Root/detail/setFileIndexPointer.h`\
`art/art/Framework/Principal/GroupFactory.cc`\
`art/art/Persistency/RootDB/SQLite3Wrapper.h`\
`art/art/Persistency/RootDB/tkeyvfs.cc`\
`art/art/Persistency/RootDB/tkeyvfs.h`\
`canvas/canvas/Persistency/Common/Assns.h`\
`canvas/canvas/Persistency/Common/CacheStreamers.cc`\
`canvas/canvas/Persistency/Common/CacheStreamers.h`\
`canvas/canvas/Persistency/Common/RefCoreStreamer.cc`\
`canvas/canvas/Persistency/Common/RefCoreStreamer.h`\
`canvas/canvas/Persistency/Common/detail/maybeCastObj.cc`\
`canvas/canvas/Persistency/Common/detail/setPtrVectorBaseStreamer.cc`\
`canvas/canvas/Persistency/Common/detail/setPtrVectorBaseStreamer.h`\
`canvas/canvas/Persistency/Provenance/BranchDescription.cc`\
`canvas/canvas/Persistency/Provenance/DictionaryChecker.cc`\
`canvas/canvas/Persistency/Provenance/TransientStreamer.h`\
`canvas/canvas/Persistency/Provenance/TypeTools.cc`\
`canvas/canvas/Persistency/Provenance/TypeTools.h`\
`canvas/canvas/Persistency/Provenance/TypeWithDict.cc`\
`canvas/canvas/Utilities/TypeID.cc`

    These files are all involved with the type system and/or "normal" ROOT I/O and are handled in a separate report.

## `art::TFileService`{.cpp} and friends.

* `art::TFileService`{.cpp} opens a new `TFile`{.cpp} file based on the provided configuration at service construction time. This is not guaranteed to be early, although in the current implementation of the framework all services are constructed at configuration validation time, before the `beginJob` stage.

* `art::TFileService`{.cpp} has only one public data member, which exposes a non-`const`{.cpp} `TFile`{.cpp} pointer to user code.

* `art::TFileService`{.cpp} takes advantage of the signals and slots mechanism to ensure that `art::TFileService::setDirectoryName(...)`{.cpp} is called to set ROOT's `gDirectory`{.cpp} global variable. This is a (kind of) thread-local variable, which could have consequences within TBB's task-based system, especially in the context of task preemption.

* `art::TFileDirectory`{.cpp} keeps track of a particular directory, and when calling functions such as `mkdir(...)`{.cpp}, `make<T, ARGS...>(...)`{.cpp} or `makeAndRegister<T, ARGS...>(...)`{.cpp} will cd into that directory for the duration of the desired operation. These functions make use of `TDirectory::GetDirectory(...)`{,cpp}, `gDirectory`{.cpp} (see above), `TFile::cd()`{.cpp}, and possibly `T::T(...)`{.cpp}, `T::SetName(...)`{.cpp}, `T::SetTitle(...)`{.cpp}, and `TDirectory::Append(...)`{.cpp}.
