#ifndef NP_FORMATTER_HPP
#define NP_FORMATTER_HPP

namespace np {
  // contract: User may specialize template, but may not overload
  // I may add overloads
  // Rationale: overloads are not dependent, so they're resolved on template definition, not
  // instantiation
  // TODO: think about handling standard types as non-template overloads, so we can push them to .cc
  // files
  template <typename T>
  void format(T&& val, Serializer& srl) {
    srl.write(val);
  }
}

#endif
