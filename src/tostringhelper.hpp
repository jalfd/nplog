#ifndef NP_TO_STRING_HELPER_HPP
#define NP_TO_STRING_HELPER_HPP

#include <algorithm>
#include <string_view>

#include <cstdint>
#include <cstring>

using DecimalString4 = uint64_t;
using DecimalString2 = uint16_t;

// what are the ops we care about?
// you load it, and then you check if the cached value matches
// if it doesn't, you create a new one from the int value, and store that
// if it does, you extract the string rep

namespace {
    /*
  constexpr uint32_t num(DecimalString4 val) { return val & 0xffffffff; }

  char* writeStr(DecimalString4 val, char* dst) {
    auto first = reinterpret_cast<char*>(val) + 4;
    memcpy(dst, first, 4);
    return first + 4;
  }

  constexpr uint8_t num(DecimalString2 val) { return val & 0xff; }
  char* writeStr(DecimalString2 val, char* dst) {
    auto first = reinterpret_cast<char*>(val) + 1;
    memcpy(dst, first, 1);
    return first + 1;
  }
  */

  const char* digits
    = "00000100200300400500600700800901001101201301401501601701801902002102202302"
      "40250260270280290300310320330340350360370380390400410420430440450460470480"
      "49050051052053054055056057058059060061062063064065066067068069070071072073"
      "07407507607707807908008108208308408508608708808909009109209309409509609709"
      "80991001011021031041051061071081091101111121131141151161171181191201211221"
      "23124125126127128129130131132133134135136137138139140141142143144145146147"
      "14814915015115215315415515615715815916016116216316416516616716816917017117"
      "21731741751761771781791801811821831841851861871881891901911921931941951961"
      "97198199200201202203204205206207208209210211212213214215216217218219220221"
      "22222322422522622722822923023123223323423523623723823924024124224324424524"
      "62472482492502512522532542552562572582592602612622632642652662672682692702"
      "71272273274275276277278279280281282283284285286287288289290291292293294295"
      "29629729829930030130230330430530630730830931031131231331431531631731831932"
      "03213223233243253263273283293303313323333343353363373383393403413423433443"
      "45346347348349350351352353354355356357358359360361362363364365366367368369"
      "37037137237337437537637737837938038138238338438538638738838939039139239339"
      "43953963973983994004014024034044054064074084094104114124134144154164174184"
      "19420421422423424425426427428429430431432433434435436437438439440441442443"
      "44444544644744844945045145245345445545645745845946046146246346446546646746"
      "84694704714724734744754764774784794804814824834844854864874884894904914924"
      "93494495496497498499500501502503504505506507508509510511512513514515516517"
      "51851952052152252352452552652752852953053153253353453553653753853954054154"
      "25435445455465475485495505515525535545555565575585595605615625635645655665"
      "67568569570571572573574575576577578579580581582583584585586587588589590591"
      "59259359459559659759859960060160260360460560660760860961061161261361461561"
      "66176186196206216226236246256266276286296306316326336346356366376386396406"
      "41642643644645646647648649650651652653654655656657658659660661662663664665"
      "66666766866967067167267367467567667767867968068168268368468568668768868969"
      "06916926936946956966976986997007017027037047057067077087097107117127137147"
      "15716717718719720721722723724725726727728729730731732733734735736737738739"
      "74074174274374474574674774874975075175275375475575675775875976076176276376"
      "47657667677687697707717727737747757767777787797807817827837847857867877887"
      "89790791792793794795796797798799800801802803804805806807808809810811812813"
      "81481581681781881982082182282382482582682782882983083183283383483583683783"
      "88398408418428438448458468478488498508518528538548558568578588598608618628"
      "63864865866867868869870871872873874875876877878879880881882883884885886887"
      "88888989089189289389489589689789889990090190290390490590690790890991091191"
      "29139149159169179189199209219229239249259269279289299309319329339349359369"
      "37938939940941942943944945946947948949950951952953954955956957958959960961"
      "96296396496596696796896997097197297397497597697797897998098198298398498598"
      "6987988989990991992993994995996997998999";
} // namespace
namespace np::log::stringhelper {
  // will return a pointer to the first non-zero digit
  template <typename T>
  constexpr inline char* fixed_unsigned_to_decimal(T number, char* buffer, size_t len) {
    char* ptr = buffer + len - 1;
    if (number == 0) {
      *ptr-- = '0';
    } else {
      while (number != 0) {
        *ptr-- = static_cast<char>('0' + static_cast<char>(number % 10));
        number /= 10;
      }
    }
    return ptr + 1;
  }

  // will return a pointer to the first non-zero digit
  // prior digits will be set to pad
  template <typename T>
  constexpr inline char* pad_unsigned_to_decimal(T number, char* buffer, size_t len, char pad) {
    const auto actual_begin = fixed_unsigned_to_decimal(number, buffer, len);
    std::fill(buffer, actual_begin, pad);
    return actual_begin;
  }

  // will return a pointer past the end of the number
  template <typename T>
  constexpr inline char* unsigned_to_decimal(T number, char* buffer, size_t len) {
    const auto actual_begin = fixed_unsigned_to_decimal(number, buffer, len);
    const auto actual_len = buffer + len - actual_begin;
    std::copy(actual_begin, buffer + len, buffer);
    return buffer + actual_len;
  }

  template <typename T>
  constexpr auto to_unsigned(T number) {
    using UT = typename std::make_unsigned<T>::type;
    auto unum = static_cast<UT>(number);
    unum = 0 - unum;
    return unum;
  }
  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the end of the buffer
  template <typename T>
  constexpr inline std::string_view decimal_from(T number, char* first, char* last) {
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* end = unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *first = '-';
        return std::string_view(first, to_size_t_checked(end - first));
      }
    }
    const auto end = unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    return std::string_view(first, to_size_t_checked(end - first));
  }

  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the beginning of the buffer
  template <typename T>
  constexpr inline std::string_view fixed_decimal_from(T number, char* first, char* last) {
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* start = fixed_unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *--start = '-';
        return std::string_view(start, to_size_t_checked(last - start));
      }
    }
    const auto start = fixed_unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    return std::string_view(start, to_size_t_checked(last - start));
  }

  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the beginning of the buffer, initialized to pad
  template <typename T>
  constexpr inline std::string_view padded_decimal_from(T number, char* first, char* last, char pad) {
    if (last - first == 2) {
      // find the right offset
      const char* digit_ptr = digits + number * 3 + 1;
      memcpy(first, digit_ptr, 2);
      return std::string_view(first, to_size_t_checked(last - first));
    }
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* start = fixed_unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *--start = '-';
        std::fill(first, start, pad);
        return std::string_view(start, to_size_t_checked(last - start));
      }
    }
    const auto start = fixed_unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    std::fill(first, start, pad);
    return std::string_view(start, to_size_t_checked(last - start));
  }
} // namespace np

#endif
