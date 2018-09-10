#include "art/Framework/Core/detail/graph_algorithms.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/graph_utility.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"

#include <limits>

using art::detail::Edge;
using art::detail::ModuleGraph;
using art::detail::ModuleGraphInfo;
using art::detail::ModuleGraphInfoMap;
using art::detail::Vertex;
using art::detail::module_name_t;
using art::detail::name_set_t;
using art::detail::path_name_t;

namespace {
  std::string
  comma_separated_list(name_set_t const& names)
  {
    if (names.empty())
      return {};

    auto cb = cbegin(names);
    std::string result{*cb};
    for (auto i = std::next(cb), e = cend(names); i != e; ++i) {
      result += ", ";
      result += *i;
    }
    return result;
  }

  inline std::string const&
  module_label(art::WorkerInPath::ConfigInfo const& wci)
  {
    return wci.moduleConfigInfo->moduleLabel;
  }
}

std::pair<ModuleGraph, std::string>
art::detail::make_module_graph(ModuleGraphInfoMap const& modInfos,
                               paths_to_modules_t const& trigger_paths,
                               configs_t const& end_path)
{
  auto const nmodules = modInfos.size();
  ModuleGraph module_graph{nmodules};

  make_trigger_path_subgraphs(modInfos, trigger_paths, module_graph);
  make_product_dependency_edges(modInfos, module_graph);
  auto err = verify_no_interpath_dependencies(modInfos, module_graph);
  if (err.empty()) {
    // Cannot currently check intrapath dependencies unless the above
    // check is successful.
    err += verify_in_order_dependencies(modInfos, trigger_paths);
  }

  make_path_ordering_edges(modInfos, trigger_paths, module_graph);
  make_synchronization_edges(modInfos, trigger_paths, end_path, module_graph);

  return std::make_pair(module_graph, err);
}

void
art::detail::make_trigger_path_subgraphs(
  ModuleGraphInfoMap const& modInfos,
  paths_to_modules_t const& trigger_paths,
  ModuleGraph& module_graph)
{
  std::map<path_name_t, ModuleGraph*> path_graphs;
  auto const source_index = modInfos.vertex_index("input_source");
  for (auto const& path : trigger_paths) {
    auto& path_graph = module_graph.create_subgraph();
    // Always include the source on the path.
    add_vertex(source_index, path_graph);
    path_graphs[path.first] = &path_graph;
  }

  auto vertex_names = get(boost::vertex_name_t{}, module_graph);
  for (auto const& pr : modInfos) {
    auto const& module_name = pr.first;
    auto const& info = pr.second;
    if (!is_modifier(info.module_type))
      continue;
    auto const index = modInfos.vertex_index(pr.first);
    for (auto const& path : info.paths) {
      add_vertex(index, *path_graphs.at(path));
    }
    vertex_names[index] = module_name;
  }
}

void
art::detail::make_product_dependency_edges(ModuleGraphInfoMap const& modInfos,
                                           ModuleGraph& graph)
{
  auto edge_label = get(boost::edge_name, graph);
  for (Vertex u{}; u < modInfos.size(); ++u) {
    auto const& modu = modInfos.info(u);
    for (auto const& dep : modu.consumed_products) {
      auto const v = modInfos.vertex_index(dep.label);
      auto const edge = add_edge(u, v, graph);
      edge_label[edge.first] = "prod";
    }
  }
}

void
art::detail::make_path_ordering_edges(ModuleGraphInfoMap const& modInfos,
                                      paths_to_modules_t const& trigger_paths,
                                      ModuleGraph& graph)
{
  // Make edges corresponding to path ordering
  auto path_label = get(boost::edge_name, graph);
  for (auto const& path : trigger_paths) {
    auto const& modules = path.second;
    if (modules.empty())
      continue;
    auto prev = cbegin(modules);
    auto curr = prev + 1;
    auto const end = cend(modules);
    while (curr != end) {
      auto const pi = modInfos.vertex_index(module_label(*prev));
      auto const ci = modInfos.vertex_index(module_label(*curr));
      auto const edge = add_edge(ci, pi, graph);
      path_label[edge.first] = "path:" + path.first;
      prev = curr;
      ++curr;
    }
  }
}

void
art::detail::make_synchronization_edges(ModuleGraphInfoMap const& modInfos,
                                        paths_to_modules_t const& trigger_paths,
                                        configs_t const& end_path,
                                        ModuleGraph& graph)
{
  auto const source_index = modInfos.vertex_index("input_source");
  auto sync_label = get(boost::edge_name, graph);
  if (!trigger_paths.empty()) {
    auto const tr_index = modInfos.vertex_index("TriggerResults");
    for (auto const& path : trigger_paths) {
      auto const& modules = path.second;
      if (modules.empty()) {
        continue;
      }
      auto const front_index =
        modInfos.vertex_index(module_label(modules.front()));
      auto const back_index =
        modInfos.vertex_index(module_label(modules.back()));
      auto const edge1 = add_edge(front_index, source_index, graph);
      sync_label[edge1.first] = "source:" + path.first;
      auto const edge2 = add_edge(tr_index, back_index, graph);
      sync_label[edge2.first] = "sync";
    }
    for (auto const& module : end_path) {
      auto const index = modInfos.vertex_index(module_label(module));
      auto const edge = add_edge(index, tr_index, graph);
      sync_label[edge.first] = "sync";
    }
  } else if (!end_path.empty()) {
    for (auto const& module : end_path) {
      auto const index = modInfos.vertex_index(module_label(module));
      auto const edge = add_edge(index, source_index, graph);
      sync_label[edge.first] = "sync";
    }
  }

  auto constexpr invalid = std::numeric_limits<std::size_t>::max();

  // Now synchronize between previous filters
  for (auto const& path : trigger_paths) {
    auto preceding_filter_index = invalid;
    for (auto const& module : path.second) {
      auto const index = modInfos.vertex_index(module_label(module));
      auto const& info = modInfos.info(index);
      if (preceding_filter_index != invalid) {
        auto const edge = add_edge(index, preceding_filter_index, graph);
        sync_label[edge.first] = "filter:" + path.first;
      }
      if (info.module_type == ModuleType::filter) {
        preceding_filter_index = index;
      }
    }
  }

  if (trigger_paths.empty()) {
    return;
  }

  // Synchronize end-path modules if a 'SelectEvents' parameter has
  // been specified.  Treat it as a filter.
  auto const tr_index = modInfos.vertex_index("TriggerResults");
  for (auto const& module : end_path) {
    auto const index = modInfos.vertex_index(module_label(module));
    auto const& info = modInfos.info(index);
    for (auto const& path : info.select_events) {
      auto const edge = add_edge(index, tr_index, graph);
      sync_label[edge.first] = "filter:" + path;
    }
  }
}

std::string
art::detail::verify_no_interpath_dependencies(
  ModuleGraphInfoMap const& modInfos,
  ModuleGraph const& graph)
{
  std::map<Vertex, std::set<Vertex>> illegal_dependencies;
  auto const children_iters = graph.children();
  for (auto ci = children_iters.first, ci_end = children_iters.second;
       ci != ci_end;
       ++ci) {
    auto const& path_graph = *ci;
    auto const vertex_iters = vertices(path_graph);
    for (auto i = vertex_iters.first, end = vertex_iters.second; i != end;
         ++i) {
      auto const gv = path_graph.local_to_global(*i);
      auto const out_edge_iters = out_edges(gv, graph);
      // Verify that the target of each out edge is a member of the
      // same subgraph (which is determined by calling find_vertex).
      // If not, then it is an illegal path specification.
      for (auto ei = out_edge_iters.first, edge_end = out_edge_iters.second;
           ei != edge_end;
           ++ei) {
        auto const tv = target(*ei, graph);
        if (path_graph.find_vertex(tv).second) {
          continue;
        }
        illegal_dependencies[gv].insert(tv);
      }
    }
  }

  if (illegal_dependencies.empty()) {
    return {};
  }

  std::ostringstream oss;
  oss << "\nThe following represent cross-path data-dependency errors:\n"
      << cet::HorizontalRule{60}('-') << '\n';
  for (auto const& mod : illegal_dependencies) {
    auto const mod_index = mod.first;
    auto const& module_name = modInfos.name(mod_index);
    auto const& mod_paths = modInfos.info(mod_index).paths;
    oss << "  Module " << module_name << " on path"
        << (mod_paths.size() == 1ull ? " " : "s ")
        << comma_separated_list(mod_paths) << " depends on\n";
    for (auto const& dep : mod.second) {
      auto const& dep_name = modInfos.name(dep);
      auto const& on_paths = modInfos.info(dep).paths;
      oss << "    Module " << dep_name << " on path"
          << (on_paths.size() == 1ull ? " " : "s ")
          << comma_separated_list(on_paths) << '\n';
    }
  }
  oss << "\nSuch errors occur whenever a module on one path depends on the "
         "data products\n"
      << "from another.  Such dependencies can be subtle--for example, a "
         "module that\n"
      << "uses an event.getManyByType call cannot be shared across paths if "
         "the modules\n"
      << "that precede it in the paths do not give consistent result.  Please "
         "check your\n"
      << "configuration, or email artists@fnal.gov for assistance.\n";
  return oss.str();
}

namespace {
  struct name_matches {
    std::string module_name;
    explicit name_matches(std::string const& name) : module_name{name} {}
    bool
    operator()(art::WorkerInPath::ConfigInfo const& info) const
    {
      return module_label(info) == module_name;
    }
  };
}

std::string
art::detail::verify_in_order_dependencies(
  ModuleGraphInfoMap const& modInfos,
  paths_to_modules_t const& trigger_paths)
{
  // Precondition: there are no inter-path dependencies
  std::map<module_name_t, name_set_t> illegal_module_orderings;
  for (auto const& module : modInfos) {
    auto const& module_name = module.first;
    auto const module_type = module.second.module_type;
    auto const& module_paths = module.second.paths;
    if (!is_modifier(module_type)) {
      continue;
    }
    if (module_paths.empty()) {
      continue;
    }
    // Only need to check one path since when we call this function,
    // we have already guaranteed that there are no interpath
    // dependencies.
    auto const& first_path_for_module = *cbegin(module_paths);
    auto const& path = trigger_paths.at(first_path_for_module);
    auto const begin = cbegin(path);
    auto const end = cend(path);
    auto const module_position =
      std::find_if(begin, end, name_matches{module_name});
    assert(module_position != end);

    for (auto const& dep : module.second.consumed_products) {
      if (dep.label == "input_source") {
        continue;
      }
      auto const dep_position =
        std::find_if(begin, end, name_matches{dep.label});
      assert(dep_position != end);
      if (dep_position < module_position) {
        continue;
      }
      illegal_module_orderings[module_name].insert(dep.label);
    }
  }

  if (illegal_module_orderings.empty()) {
    return {};
  }

  std::ostringstream oss;
  oss << "\nThe following are module-ordering errors due to declared "
         "data-product dependencies:\n";
  for (auto const& mod : illegal_module_orderings) {
    auto const& module_name = mod.first;
    auto const mod_index = modInfos.vertex_index(module_name);
    auto const& module_paths = modInfos.info(mod_index).paths;
    oss << "  Module " << module_name << " on path"
        << (module_paths.size() == 1ull ? " " : "s ")
        << comma_separated_list(module_paths)
        << " depends on either itself or modules that follow it:\n";
    for (auto const& dep_name : mod.second) {
      auto const dep_index = modInfos.vertex_index(dep_name);
      auto const& on_paths = modInfos.info(dep_index).paths;
      oss << "    Module " << dep_name << " on path"
          << (on_paths.size() == 1ull ? " " : "s ")
          << comma_separated_list(on_paths)
          << (module_name == dep_name ? " (self circularity)" : "") << '\n';
    }
  }
  return oss.str();
}

using EdgePair = std::pair<Vertex, Vertex>;

namespace {
  class graph_printer : public boost::dfs_visitor<> {
  public:
    explicit graph_printer(
      ModuleGraphInfoMap const& modules,
      std::set<Vertex>& vertices,
      std::map<path_name_t, std::set<EdgePair>>& path_edges,
      std::map<path_name_t, std::set<EdgePair>>& filter_edges,
      std::set<EdgePair>& trigger_path_edges,
      std::map<EdgePair, unsigned>& product_edges)
      : modules_{modules}
      , vertices_{vertices}
      , path_edges_{path_edges}
      , filter_edges_{filter_edges}
      , trigger_path_edges_{trigger_path_edges}
      , product_edges_{product_edges}
    {}

    void
    discover_vertex(Vertex const v, ModuleGraph const&)
    {
      vertices_.insert(v);
    }

    void
    forward_or_cross_edge(Edge const e, ModuleGraph const& g)
    {
      print_edge(e, g);
    }

    void
    tree_edge(Edge const e, ModuleGraph const& g)
    {
      print_edge(e, g);
    }

    void
    back_edge(Edge const e, ModuleGraph const& g)
    {
      print_edge(e, g);
    }

  private:
    enum class arrow_style { path, sync, source, filter, prod };

    static arrow_style
    arrow_style_for(std::string const& edge_name)
    {
      auto pos = edge_name.find("path:");
      if (pos == 0) {
        return arrow_style::path;
      }
      pos = edge_name.find("sync");
      if (pos == 0) {
        return arrow_style::sync;
      }
      pos = edge_name.find("source:");
      if (pos == 0) {
        return arrow_style::source;
      }
      pos = edge_name.find("filter:");
      if (pos == 0) {
        return arrow_style::filter;
      }
      if (edge_name == "prod") {
        return arrow_style::prod;
      }
      throw art::Exception{
        art::errors::LogicError,
        "An occurred while printing the DOT file for this process.\n"}
        << "The edge name '" << edge_name << "' is not recognized.";
    }

    void
    print_edge(Edge const e, ModuleGraph const& g)
    {
      auto const u = source(e, g);
      auto const v = target(e, g);
      auto const vertex_pair = std::make_pair(u, v);
      auto const& edge_name = get(boost::edge_name, g, e);
      switch (arrow_style_for(edge_name)) {
        case arrow_style::path: {
          auto path_name = edge_name.substr(5); // Removes 'path:' prefix
          path_edges_[path_name].insert(vertex_pair);
          break;
        }
        case arrow_style::sync: {
          trigger_path_edges_.insert(vertex_pair);
          break;
        }
        case arrow_style::source: {
          auto path_name = edge_name.substr(7); // Remove 'source:' prefix
          path_edges_[path_name].insert(vertex_pair);
          break;
        }
        case arrow_style::filter: {
          auto path_name = edge_name.substr(7); // Removes 'filter:' prefix
          filter_edges_[path_name].insert(vertex_pair);
          break;
        }
        case arrow_style::prod:
          ++product_edges_[vertex_pair];
      }
    }

    ModuleGraphInfoMap const& modules_
#if CLANG_IS_AT_LEAST(6, 0, 0)
      [[maybe_unused]]
#endif
      ;
    std::set<Vertex>& vertices_;
    std::map<path_name_t, std::set<EdgePair>>& path_edges_;
    std::map<path_name_t, std::set<EdgePair>>& filter_edges_;
    std::set<EdgePair>& trigger_path_edges_;
    std::map<EdgePair, unsigned>& product_edges_;
  };
}

void
art::detail::print_module_graph(std::ostream& os,
                                ModuleGraphInfoMap const& info_map,
                                ModuleGraph const& graph)
{
  std::set<Vertex> vertices;
  std::map<path_name_t, std::set<EdgePair>> path_edges;
  std::map<path_name_t, std::set<EdgePair>> filter_edges;
  std::set<EdgePair> trigger_path_edges;
  std::map<EdgePair, unsigned> product_edges;
  graph_printer printer{info_map,
                        vertices,
                        path_edges,
                        filter_edges,
                        trigger_path_edges,
                        product_edges};
  boost::depth_first_search(graph, visitor(printer));
  os << "digraph {\n"
     << "  rankdir=BT\n";
  // Vertices
  for (auto const& v : vertices) {
    auto const& name = info_map.name(v);
    auto const& info = info_map.info(v);
    if (name == "input_source") {
      os << "  \"input_source\"[shape=box label=source]";
    } else if (name == "TriggerResults") {
      os << "  \"" << name << '\"';
      os << "[shape=box style=filled fillcolor=black label=\"\" height=0.1 "
            "width=2]";
    } else {
      os << "  \"" << name << '\"';
      if (info.module_type == art::ModuleType::filter) {
        os << "[style=filled fillcolor=pink]";
      }
    }
    os << ";\n";
  }

  // Path edges
  for (auto const& pr : path_edges) {
    auto const& path_name = pr.first;
    for (auto const& edge_pair : pr.second) {
      os << "  \"" << info_map.name(edge_pair.first) << "\" -> \""
         << info_map.name(edge_pair.second) << '\"' << "[label=\"" << path_name
         << "\" color=gray];\n";
    }
  }

  // Filter edges
  for (auto const& pr : filter_edges) {
    auto const& path_name = pr.first;
    for (auto const& edge_pair : pr.second) {
      os << "  \"" << info_map.name(edge_pair.first) << "\" -> \""
         << info_map.name(edge_pair.second) << '\"' << "[label=\"" << path_name
         << "\" color=red];\n";
    }
  }

  // Trigger-path edges
  for (auto const& edge_pair : trigger_path_edges) {
    os << "  \"" << info_map.name(edge_pair.first) << "\" -> \""
       << info_map.name(edge_pair.second) << '\"'
       << "[style=invisible arrowhead=none];\n";
  }

  // Product edges
  for (auto const& pr : product_edges) {
    auto const& edge_pair = pr.first;
    auto const multiplicity = pr.second;
    os << "  \"" << info_map.name(edge_pair.first) << "\" -> \""
       << info_map.name(edge_pair.second) << '\"';
    if (multiplicity > 1) {
      os << "[label=\"" << multiplicity << "\" color=black]";
    }
    os << ";\n";
  }
  os << "}\n";
}
