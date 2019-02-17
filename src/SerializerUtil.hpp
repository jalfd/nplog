namespace np {
    template <typename BufType>
    struct JsonBuilder {
      size_t beginObject(char* insert)
      {
          *insert = '{';
          return 1;
      }

      size_t endObject(char* insert)
      {
          *insert = '}';
          return 1;
      }

      size_t beginArray(char* insert)
      {
          *insert = '[';
          return 1;
      }

      size_t endArray(char* insert)
      {
          *insert = ']';
          return 1;
      }

      size_t separator(char* insert)
      {
          *insert = ',';
          return 1;
      }

      virtual void writeString();
      virtual void writeNumber();
      virtual void writeBool(char* insert, bool val) {
          if (val) {
              std::copy_n("true", 4, insert);
              return 4;
          } else {
              std::copy_n("false", 5, insert);
              return 5;
          }
      }

      template <typename T>
      size_t maxLength(const T& val) {
          // Numbers can be formatted in 20-something characters. Round up a bit because why not
          return 32;
      }

      template <>
      size_t maxLength<std::string_view>(const T& val) {
        // at worst, each character must be escaped as \uXXXX. Additionally, strings are wrapped in
        // quotes
        return val.size() * 6 + 2;
      }

      template <>
      size_t maxLength<bool>(const T& val) {
          return 5;
      }

    };
}
