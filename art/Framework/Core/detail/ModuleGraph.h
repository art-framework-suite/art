#ifndef art_Framework_Core_detail_ModuleGraph_h
#define art_Framework_Core_detail_ModuleGraph_h

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/subgraph.hpp"

#include <string>

namespace art::detail {
  using vertex_property = boost::property<boost::vertex_name_t, std::string>;
  using edge_property =
    boost::property<boost::edge_index_t,
                    int,
                    boost::property<boost::edge_name_t, std::string>>;
  using graph_property = boost::property<boost::graph_name_t, std::string>;
  using Graph = boost::adjacency_list<boost::vecS,
                                      boost::vecS,
                                      boost::bidirectionalS,
                                      vertex_property,
                                      edge_property,
                                      graph_property>;
  using ModuleGraph = boost::subgraph<Graph>;
  using Edge = boost::graph_traits<ModuleGraph>::edge_descriptor;
  using Vertex = boost::graph_traits<ModuleGraph>::vertex_descriptor;
}

#endif /* art_Framework_Core_detail_ModuleGraph_h */

// Local Variables:
// mode: c++
// End:
