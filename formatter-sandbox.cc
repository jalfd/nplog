// Deseried api:
serializer api should be something like this : serializer::format(iter, double);
serializer::format(iter, string);
serializer::format(iter, bool);
ScopedObject serializer::object(iter);
ScopedObject::ScopedProperty ScopedObject::property(name);
ScopedArray serializer::array(iter);
ScopedArray::Appender ScopedArray::appender();

namespace np::log {
    template <typename OutIter>
    void format(OutIter iter, type T);
}

// example UDT:
struct Coordinate {
    double xyz[3];
};

// example format functions
void format(OutIter iter, Coordinate c) {
  const auto arr = literalArray(iter);
  for (const auto elem : c.xyx) {
    const auto elem = arr.appender();
    format(iter, elem);
  }
}

void format(OutIter iter, Coordinate c) {
  const auto arr = literalObject(iter);
  {
    const auto elem = arr.property("x");
    format(iter, c.xyz[0]);
  }
  {
    const auto elem = arr.property("y");
    format(iter, c.xyz[1]);
  }
  {
    const auto elem = arr.property("z");
    format(iter, c.xyz[2]);
  }
}


