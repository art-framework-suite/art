// ======================================================================
//
// FileDumperOutput.cc: "dump contents of a file"
//
// Proposed output format (Feature #941):
// Process Name | Module Label | Process Label | Data Product type | (ProductID
// |) size
//
// ======================================================================

#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "range/v3/view.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace ::ranges;

namespace art::detail {
  struct ProductInfo {
    std::string module_label;
    std::string instance_name;
    std::string product_type;
    std::string friendly_type;
    std::string product_id;
    std::string str_size;
  };
} // namespace art::detail

namespace {

  std::string
  product_size(art::EDProduct const* product, bool const isPresent)
  {
    return isPresent ? product->productSize() : "?";
  }

  std::string
  dummyProcess()
  {
    return "PROCESS NAME";
  }

  auto
  dummyInfo()
  {
    return art::detail::ProductInfo{"MODULE LABEL",
                                    "PRODUCT INSTANCE NAME",
                                    "DATA PRODUCT TYPE",
                                    "PRODUCT FRIENDLY TYPE",
                                    "PRODUCT ID",
                                    "SIZE"};
  }

  using ProductInfos = std::vector<art::detail::ProductInfo>;
  std::size_t
  columnWidthFirst(std::map<std::string, ProductInfos> const& m,
                   std::string const& title)
  {
    std::size_t i{title.size()};
    cet::for_all(
      m, [&i](auto const& entry) { i = std::max(i, entry.first.size()); });
    return i;
  }

  std::size_t
  columnWidth(std::map<std::string, ProductInfos> const& m,
              std::string const art::detail::ProductInfo::*pim,
              std::string const& title)
  {
    std::size_t i{title.size()};
    for (auto const& entry : m) {
      for (auto const& pi : entry.second) {
        i = std::max(i, (pi.*pim).size());
      }
    }
    return i;
  }

} // namespace

namespace art {
  class FileDumperOutput;
}

class art::FileDumperOutput : public OutputModule {
public:
  struct Config {
    fhicl::TableFragment<OutputModule::Config> omConfig;
    fhicl::Atom<bool> wantProductFullClassName{
      fhicl::Name("wantProductFullClassName"),
      true};
    fhicl::Atom<bool> wantProductFriendlyClassName{
      fhicl::Name("wantProductFriendlyClassName"),
      wantProductFullClassName()};
    fhicl::Atom<bool> wantProductID{fhicl::Name{"wantProductID"}, false};
    fhicl::Atom<bool> resolveProducts{fhicl::Name("resolveProducts"), true};
    fhicl::Atom<bool> onlyIfPresent{fhicl::Name("onlyIfPresent"), false};
  };

  using Parameters =
    fhicl::WrappedTable<Config, OutputModule::Config::KeysToIgnore>;

  explicit FileDumperOutput(Parameters const&);

private:
  void write(EventPrincipal& e) override;
  void writeRun(RunPrincipal& r) override;
  void writeSubRun(SubRunPrincipal& sr) override;
  void readResults(ResultsPrincipal const& resp) override;

  template <typename P>
  void printPrincipal(P const& p);

  void printProductInfo(std::vector<std::size_t> const& columnWidths,
                        std::string const& processName,
                        detail::ProductInfo const& pi) const;

  bool const wantProductFullClassName_;
  bool const wantProductFriendlyClassName_;
  bool const wantProductID_;
  bool const wantResolveProducts_;
  bool const wantPresentOnly_;
}; // FileDumperOutput

art::FileDumperOutput::FileDumperOutput(Parameters const& ps)
  : OutputModule{ps().omConfig}
  , wantProductFullClassName_{ps().wantProductFullClassName()}
  , wantProductFriendlyClassName_{ps().wantProductFriendlyClassName()}
  , wantProductID_{ps().wantProductID()}
  , wantResolveProducts_{ps().resolveProducts()}
  , wantPresentOnly_{ps().onlyIfPresent()}
{}

void
art::FileDumperOutput::write(EventPrincipal& e)
{
  printPrincipal(e);
}

void
art::FileDumperOutput::writeRun(RunPrincipal& r)
{
  printPrincipal(r);
}

void
art::FileDumperOutput::writeSubRun(SubRunPrincipal& sr)
{
  printPrincipal(sr);
}

void
art::FileDumperOutput::readResults(ResultsPrincipal const& resp)
{
  printPrincipal(resp);
}

template <typename P>
void
art::FileDumperOutput::printPrincipal(P const& p)
{
  if (!p.size())
    return;

  size_t present{0};
  size_t not_present{0};
  std::map<std::string, std::vector<detail::ProductInfo>> products;

  auto const& dinfo = dummyInfo();

  products[dummyProcess()].emplace_back(dinfo);

  for (auto const& g : p | views::values | views::indirect) {
    auto const& pd = g.productDescription();
    auto const& oh = p.getForOutput(pd.productID(), wantResolveProducts_);

    EDProduct const* product = oh.isValid() ? oh.wrapper() : nullptr;
    bool const productPresent = product != nullptr && product->isPresent();

    if (productPresent) {
      ++present;
    } else {
      ++not_present;
    }

    if (!wantPresentOnly_ || productPresent) {
      auto pi = detail::ProductInfo{pd.moduleLabel(),
                                    pd.productInstanceName(),
                                    pd.producedClassName(),
                                    pd.friendlyClassName(),
                                    std::to_string(pd.productID().value()),
                                    product_size(product, productPresent)};
      products[pd.processName()].emplace_back(std::move(pi));
    }
  }

  std::cout << "PRINCIPAL TYPE: " << BranchTypeToString(p.branchType())
            << std::endl;

  std::vector<std::size_t> const widths{
    columnWidthFirst(products, dummyProcess()),
    columnWidth(
      products, &detail::ProductInfo::module_label, dinfo.module_label),
    columnWidth(
      products, &detail::ProductInfo::instance_name, dinfo.instance_name),
    columnWidth(
      products, &detail::ProductInfo::product_type, dinfo.product_type),
    columnWidth(
      products, &detail::ProductInfo::friendly_type, dinfo.friendly_type),
    columnWidth(products, &detail::ProductInfo::product_id, dinfo.product_id),
    columnWidth(products, &detail::ProductInfo::str_size, dinfo.str_size)};

  // Print banner
  printProductInfo(widths, dummyProcess(), dummyInfo());
  for (auto const& processConfig : p.processHistory()) {
    auto const& processName = processConfig.processName();
    for (auto const& pi : products[processName]) {
      printProductInfo(widths, processName, pi);
    }
  }

  std::cout << "\nTotal products (present, not present): "
            << present + not_present << " (" << present << ", " << not_present
            << ").\n\n";
}

void
art::FileDumperOutput::printProductInfo(std::vector<std::size_t> const& widths,
                                        std::string const& processName,
                                        detail::ProductInfo const& pi) const
{
  std::ostringstream oss;
  oss << cet::rpad(processName, widths[0], '.') << " | "
      << cet::rpad(pi.module_label, widths[1], '.') << " | "
      << cet::rpad(pi.instance_name, widths[2], '.') << " | ";
  if (wantProductFullClassName_) {
    oss << cet::rpad(pi.product_type, widths[3], '.') << " | ";
  }
  if (wantProductFriendlyClassName_) {
    oss << cet::rpad(pi.friendly_type, widths[4], '.') << " | ";
  }
  if (wantProductID_) {
    oss << cet::rpad(pi.product_id, widths[5], '.') << " | ";
  }
  oss << cet::lpad(pi.str_size, widths[6], '.');
  std::cout << oss.str() << '\n';
}

DEFINE_ART_MODULE(art::FileDumperOutput)
