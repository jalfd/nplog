#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP

#include <nplog/Formatter.hpp>
#include <nplog/Serializer.hpp>
#include <nplog/export.hpp>

#include <string_view>
#include <vector>

namespace np {
  struct NPLOG_EXPORT Log {
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer;

    bool suppressMessage(int level) const;
    int paramLevel() const;

    buffer_type acquireBuffer();

    void submitMessage(int level, const buffer_type& buffer);

    static void setSink(std::function<void(int, std::string_view msg)> sink);

    void releaseBuffer(buffer_type&& buf);

  private:
    std::mutex buffer_mutex;
    std::vector<buffer_type> buffers;
  };

  NPLOG_EXPORT std::function<void(int, std::string_view)> getStdErrSink();
} // namespace np
#endif
