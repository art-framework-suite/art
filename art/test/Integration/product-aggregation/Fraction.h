#ifndef art_test_Integration_product_aggregation_Fraction_h
#define art_test_Integration_product_aggregation_Fraction_h

namespace arttest {

  class Fraction {
  public:
    Fraction(std::vector<unsigned> const& nums,
             std::vector<unsigned> const& denoms)
      : nums_{nums}, denoms_{denoms}
    {}

    double
    front() const
    {
      if (denoms_.empty() || nums_.empty())
        throw art::Exception(art::errors::LogicError,
                             "arttest::Fraction::front")
          << "Attempt to access front when numerator or denominator vectors "
             "were empty\n.";
      double const num = nums_[0];
      double const denom = denoms_[0];
      return denom == 0. ? 0. : num / denom;
    }

    double
    value() const
    {
      if (denoms_.size() == 0)
        return 0.;
      double const num = numerator();
      double const denom = denominator();
      return denom == 0. ? 0. : num / denom;
    }

    double
    numerator() const
    {
      return std::accumulate(nums_.begin(), nums_.end(), 0);
    }

    double
    denominator() const
    {
      return std::accumulate(denoms_.begin(), denoms_.end(), 0);
    }

    void
    pop_front()
    {
      if (denoms_.empty() || nums_.empty())
        throw art::Exception(art::errors::LogicError,
                             "arttest::Fraction::pop_front")
          << "Attempt to pop front when numerator or denominator vectors were "
             "empty\n.";
      nums_.erase(nums_.begin());
      denoms_.erase(denoms_.begin());
    }

  private:
    std::vector<unsigned> nums_;
    std::vector<unsigned> denoms_;
  };

} // namespace arttest

#endif /* art_test_Integration_product_aggregation_Fraction_h */

// Local variables:
// mode: c++
// End:
