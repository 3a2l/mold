#include "mold.h"

#include <limits>

template <typename E>
void InputSection<E>::write_to(Context<E> &ctx, u8 *buf) {
  if (shdr.sh_type == SHT_NOBITS || shdr.sh_size == 0)
    return;

  // Copy data
  memcpy(buf, contents.data(), contents.size());

  // Apply relocations
  if (shdr.sh_flags & SHF_ALLOC)
    apply_reloc_alloc(ctx, buf);
  else
    apply_reloc_nonalloc(ctx, buf);

  // As a special case, .ctors and .dtors section contents are
  // reversed. These sections are now obsolete and mapped to
  // .init_array and .fini_array, but they have to be reversed to
  // maintain the original semantics.
  bool init_fini = output_section->name == ".init_array" ||
                   output_section->name == ".fini_array";
  bool ctors_dtors = name().starts_with(".ctors") ||
                     name().starts_with(".dtors");
  if (init_fini && ctors_dtors)
    std::reverse((typename E::WordTy *)buf,
                 (typename E::WordTy *)(buf + shdr.sh_size));
}

template <typename E>
static i64 get_output_type(Context<E> &ctx) {
  if (ctx.arg.shared)
    return 0;
  if (ctx.arg.pie)
    return 1;
  return 2;
}

template <typename E>
static i64 get_sym_type(Context<E> &ctx, Symbol<E> &sym) {
  if (sym.is_absolute(ctx))
    return 0;
  if (!sym.is_imported)
    return 1;
  if (sym.get_type() != STT_FUNC)
    return 2;
  return 3;
}

template <typename E>
void InputSection<E>::dispatch(Context<E> &ctx, Action table[3][4],
                               u16 rel_type, i64 i) {
  std::span<ElfRel<E>> rels = get_rels(ctx);
  const ElfRel<E> &rel = rels[i];
  Symbol<E> &sym = *file.symbols[rel.r_sym];
  bool is_readonly = !(shdr.sh_flags & SHF_WRITE);
  Action action = table[get_output_type(ctx)][get_sym_type(ctx, sym)];

  switch (action) {
  case ACTION_NONE:
    rel_types[i] = rel_type;
    return;
  case ACTION_ERROR:
    break;
  case ACTION_COPYREL:
    if (!ctx.arg.z_copyreloc)
      break;
    if (sym.esym().st_visibility == STV_PROTECTED)
      Error(ctx) << *this << ": cannot make copy relocation for "
                 << " protected symbol '" << sym << "', defined in "
                 << *sym.file;
    sym.flags |= NEEDS_COPYREL;
    rel_types[i] = rel_type;
    return;
  case ACTION_PLT:
    sym.flags |= NEEDS_PLT;
    rel_types[i] = rel_type;
    return;
  case ACTION_DYNREL:
    if (is_readonly)
      break;
    sym.flags |= NEEDS_DYNSYM;
    rel_types[i] = R_DYN;
    file.num_dynrel++;
    return;
  case ACTION_BASEREL:
    if (is_readonly)
      break;
    rel_types[i] = R_BASEREL;
    file.num_dynrel++;
    return;
  default:
    unreachable(ctx);
  }

  Error(ctx) << *this << ": " << rel_to_string<E>(rel.r_type)
             << " relocation against symbol `" << sym
             << "' can not be used; recompile with -fPIE";
}

template class InputSection<X86_64>;
template class InputSection<I386>;
