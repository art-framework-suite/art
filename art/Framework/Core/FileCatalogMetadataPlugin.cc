#include "art/Framework/Core/FileCatalogMetadataPlugin.h"

std::string const
  cet::PluginTypeDeducer<art::FileCatalogMetadataPlugin>::value =
    "FileCatalogMetadataPlugin";

art::FileCatalogMetadataPlugin::FileCatalogMetadataPlugin(
  fhicl::ParameterSet const&)
{}
