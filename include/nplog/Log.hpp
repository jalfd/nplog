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

    void submitMessage(const buffer_type &buffer);

    void releaseBuffer(buffer_type&& buf);

  private:
    std::mutex buffer_mutex;
    std::vector<buffer_type> buffers;
  };
} // namespace np
#endif
