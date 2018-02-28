#include "art/Framework/Core/detail/ModuleGraph.h"
#include "art/Framework/Core/detail/ModuleToPath.h"
#include "art/Framework/Core/detail/graph_algorithms.h"
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/graph_utility.hpp"

using art::detail::Edge;
using art::detail::ModuleToPath;
using art::detail::ModuleGraph;
using art::detail::Vertex;

namespace {

  using cycles_t = std::vector<std::pair<Vertex, Vertex>>;
  struct cycle_detector : public boost::dfs_visitor<>
  {
    cycle_detector(cycles_t& cycles) : cycles_{cycles} {}

    void
    back_edge(Edge const e, ModuleGraph const& g)
    {
      cycles_.emplace_back(source(e, g), target(e, g));
    }

  private:
    cycles_t& cycles_;
  };

  std::string comma_separated_list(std::vector<std::string> const& names)
  {
    if (names.empty()) return {};

    std::string result{names[0]};
    for (auto i = cbegin(names)+1, e = cend(names); i != e; ++i) {
      result += ", ";
      result += *i;
    }
    return result;
  }
}

ModuleGraph
art::detail::make_module_graph(ModuleToPath const& mod_to_path,
                               paths_to_modules_t const& paths,
                               std::map<std::string, std::set<std::string>> const& deps)
{
  auto const nmodules = mod_to_path.size();
  ModuleGraph graph{nmodules};

  art::detail::make_path_graphs(mod_to_path, paths, graph);
  art::detail::make_edges_path_orderings(mod_to_path, paths, graph);
  art::detail::make_edges_data_dependencies(mod_to_path, deps, graph);
  return graph;
}


void
art::detail::make_path_graphs(ModuleToPath const& mod_to_path,
                              paths_to_modules_t const& paths,
                              ModuleGraph& graph)
{
  auto vertex_names = get(boost::vertex_name_t{}, graph);
  for (auto const& path : paths) {
    auto& path_graph = graph.create_subgraph();
    get_property(path_graph, boost::graph_name) = path.first;
    for (auto const& mod : path.second) {
      auto const i = mod_to_path.find_vertex_index(mod);
      add_vertex(i, path_graph);
      vertex_names[i] = mod;
    }
  }
}

void
art::detail::make_edges_path_orderings(ModuleToPath const& mod_to_path,
                                       paths_to_modules_t const& paths,
                                       ModuleGraph& graph)
{
  // Make edges corresponding to path ordering
  for (auto const& path : paths) {
    auto const& modules = path.second;
    if (modules.empty()) continue;
    auto prev = cbegin(modules);
    auto curr = prev+1;
    auto const end = cend(modules);
    while (curr != end) {
      auto const pi = mod_to_path.find_vertex_index(*prev);
      auto const ci = mod_to_path.find_vertex_index(*curr);
      add_edge(ci, pi, graph);
      prev = curr;
      ++curr;
    }
  }
}

void
art::detail::make_edges_data_dependencies(ModuleToPath const& mod_to_path,
                                          std::map<std::string, std::set<std::string>> const& deps,
                                          ModuleGraph& graph)
{
  for (auto vp = vertices(graph); vp.first != vp.second; ++vp.first) {
    auto const vi = *vp.first;
    auto const& mod_name = mod_to_path.name(vi);
    auto const result = deps.find(mod_name);
    if (result == cend(deps)) {
      continue;
    }
    auto const& deps = result->second;
    for (auto const& dep : deps) {
      auto const vj = mod_to_path.find_vertex_index(dep);
      add_edge(vi, vj, graph);
    }
  }
}

std::string
art::detail::verify_no_interpath_dependencies(ModuleToPath const& mod_to_path,
                                              ModuleGraph const& graph)
{
  std::map<Vertex, std::set<Vertex>> illegal_dependencies;
  auto const children_iters = graph.children();
  for (auto ci = children_iters.first, ci_end = children_iters.second; ci != ci_end; ++ci) {
    auto const& path_graph = *ci;
    auto const vertex_iters = vertices(path_graph);
    for (auto i = vertex_iters.first, end = vertex_iters.second; i != end; ++i) {
      auto const gv = path_graph.local_to_global(*i);
      auto const out_edge_iters = out_edges(gv, graph);
      // Verify that the target of each out edge is a member of the
      // same subgraph (which is determined by calling find_vertex).
      // If not, then it is an illegal path specification.
      for (auto ei = out_edge_iters.first, edge_end = out_edge_iters.second; ei != edge_end; ++ei) {
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
  oss << "The following represent data-dependency errors:\n";
  for (auto const& mod : illegal_dependencies) {
    auto const mod_index = mod.first;
    oss << "  Module " << mod_to_path.name(mod_index) << " on path(s) "
        << comma_separated_list(mod_to_path.paths(mod_index)) << " depends on\n";
    for (auto const& dep : mod.second) {
      auto const& dep_name = mod_to_path.name(dep);
      auto const& on_paths = mod_to_path.paths(dep);
      oss << "    Module " << dep_name
          << " on path(s) "
          << comma_separated_list(on_paths) << '\n';
    }
  }
  return oss.str();
}

std::string
art::detail::verify_no_circular_dependencies(ModuleToPath const& mod_to_path,
                                             ModuleGraph const& graph)
{
  std::ostringstream oss;
  auto const children_iters = graph.children();
  for (auto ci = children_iters.first, ci_end = children_iters.second; ci != ci_end; ++ci) {
    auto const& path_graph = *ci;
    cycles_t cycles{};
    cycle_detector vis{cycles};
    boost::depth_first_search(path_graph, visitor(vis));
    if (cycles.empty()) continue;

    oss << "Path " << get_property(path_graph, boost::graph_name)
        << " has the following data-dependency cycles:\n";
    for (auto const& pr : cycles) {
      auto const gu = path_graph.local_to_global(pr.first);
      auto const gv = path_graph.local_to_global(pr.second);
      oss << "  " << mod_to_path.name(gu) << " <--> " << mod_to_path.name(gv)
          << '\n';
    }
  }
  return oss.str();
}

namespace {
  struct graph_printer : public boost::dfs_visitor<>
  {
    explicit graph_printer(std::ostream& os,
                           ModuleToPath const& mod_to_path)
      : os_{os}
      , modToPath_{mod_to_path}
    {}

    void
    discover_vertex(Vertex const v, ModuleGraph const&)
    {
      os_ << "  " << modToPath_.name(v)
          << ";\n";
    }

    void
    tree_edge(Edge const e, ModuleGraph const& g)
    {
      auto const u = source(e, g);
      auto const v = target(e, g);
      os_ << "  " << modToPath_.name(u)
          << " -> " << modToPath_.name(v)
          << ";\n";
    }

    void
    back_edge(Edge const e, ModuleGraph const& g)
    {
      auto const u = source(e, g);
      auto const v = target(e, g);
      os_ << "  " << modToPath_.name(u)
          << " -> " << modToPath_.name(v)
          << ";\n";
    }

  private:
    std::ostream& os_;
    ModuleToPath const& modToPath_;
  };
}

void
art::detail::print_module_graph(std::ostream& os,
                                ModuleToPath const& mod_to_path,
                                ModuleGraph const& graph)
{
  os << "digraph {\n"
     << "  rankdir=BT\n";
  graph_printer vis{os, mod_to_path};
  boost::depth_first_search(graph, visitor(vis));
  os << "}\n";
}
