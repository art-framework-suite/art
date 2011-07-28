set(art_Framework_Services_UserInteraction_sources
  UserInteraction.cc
)

add_library(art_Framework_Services_UserInteraction
  SHARED
  ${art_Framework_Services_UserInteraction_sources}
)

target_link_libraries(art_Framework_Services_UserInteraction
  art_Framework_Services_Registry
  ${FHICLCPP}
)

install ( TARGETS art_Framework_Services_UserInteraction
  DESTINATION ${flavorqual_dir}/lib )

simple_plugin(SimpleInteraction "service" art_Framework_Services_UserInteraction)

install_headers()
install_source()
