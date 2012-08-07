set(art_Framework_Services_UserInteraction_sources
  UserInteraction.cc
)

add_library(art_Framework_Services_UserInteraction
  SHARED
  ${art_Framework_Services_UserInteraction_sources}
)

target_link_libraries(art_Framework_Services_UserInteraction
  art_Framework_Core
  art_Framework_Principal
  art_Persistency_Provenance
  art_Framework_Services_Registry
  ${FHICLCPP}
)

install ( TARGETS art_Framework_Services_UserInteraction
  DESTINATION ${flavorqual_dir}/lib )

simple_plugin(SimpleInteraction "service" art_Framework_Services_UserInteraction)
simple_plugin(AdhocFileTransfer "service")
simple_plugin(AdhocFileDelivery "service")
install_headers()
install_source()
