#include "mold.h"

#ifdef __clang__
#include <cxxabi.h>
#elif defined _MSC_VER
#include <dbghelp.h>
#endif
#include <stdlib.h>

static thread_local char *demangle_buf;
static thread_local size_t demangle_buf_len;

static bool is_mangled_name(std::string_view name) {
#ifdef __clang__
  return name.starts_with("_Z");
#elif defined _MSC_VER
  return name.starts_with("?");
#endif
}

template <typename E>
std::string_view Symbol<E>::get_demangled_name() const {
  if (is_mangled_name(name())) {
    char *mangled = new char[name().size() + 1];
    memcpy(mangled, name().data(), name().size());
    mangled[name().size()] = '\0';

#ifdef __clang__
    size_t len = sizeof(demangle_buf);
    int status;
    demangle_buf =
      abi::__cxa_demangle(mangled, demangle_buf, &demangle_buf_len, &status);
    delete[](mangled);
    if (status == 0)
      return demangle_buf;
#elif defined _MSC_VER
    const DWORD status = ::UnDecorateSymbolName(mangled, demangle_buf, demangle_buf_len, UNDNAME_COMPLETE);
    delete[](mangled);
    if (status != 0)
      return demangle_buf;
#endif
  }

  return name();
}

template <typename E>
std::ostream &operator<<(std::ostream &out, const Symbol<E> &sym) {
  if (opt_demangle)
    out << sym.get_demangled_name();
  else
    out << sym.name();
  return out;
}

#define INSTANTIATE(E)                                                  \
  template class Symbol<E>;                                             \
  template std::ostream &operator<<(std::ostream &, const Symbol<E> &)

INSTANTIATE(X86_64);
INSTANTIATE(I386);
