#ifndef art_Utilities_ostream_handle_h
#define art_Utilities_ostream_handle_h

#include <fstream>
#include <ostream>
#include <string>

namespace art {

  class ostream_handle {
  public:

    virtual ~ostream_handle(){}

    template <typename T>
    ostream_handle& operator<<(T const& t)
    {
      get_stream() << t;
      return *this;
    }

    std::ostream& stream() { return get_stream(); }

  private:

    virtual std::ostream& get_stream() = 0;

  };

  class ostream_observer : public ostream_handle {
  public:

    ostream_observer(std::ostream& os) : os_{os} {}

  private:

    std::ostream& os_;
    std::ostream& get_stream() override { return os_; }

  };

  class ostream_owner : public ostream_handle {
  public:

    ostream_owner(std::string const& fn) : ofs_{fn} {}
    ~ostream_owner() override { ofs_.close(); }

  private:

    std::ofstream ofs_;
    std::ostream& get_stream() override { return ofs_; }

  };

}

#endif

// Local variables:
// mode: c++
// End:
