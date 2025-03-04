#include <ranges>
#include <algorithm>
#include <functional>
#include "tfit/tfit.h"

// https://github.com/Neodymium146/gta-toolkit/blob/master/RageLib.GTA5/Cryptography/GTA5Encryption.cs
// https://github.com/0x1F9F1/Swage/blob/master/src/crypto/tfit.cpp

// 00000001455118F0: Encrypt200
// 00000001455119FA: call Encrypt10
// 0000000145517A60: Encrypt10

template<size_t Rounds>
TFIT<Rounds>::TFIT(std::array<uint8_t, 16> &previous, std::span<const std::array<std::array<uint8_t, 4>, 4>, Rounds> keys, std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, Rounds> &tables)
  : keys_(keys)
  , tables_(tables)
  , previous_(previous) {

}

template<size_t Rounds>
void TFIT<Rounds>::RoundA(std::array<uint8_t, 16> &data, const std::array<std::array<uint8_t, 4>, 4> &key, std::array<std::array<std::array<uint8_t, 4>, 256>, 16> &table) {
  std::array<uint32_t, 4> temp = {
    *reinterpret_cast<uint32_t *>(table[0][data[0]].data()) ^
    *reinterpret_cast<uint32_t *>(table[1][data[1]].data()) ^
    *reinterpret_cast<uint32_t *>(table[2][data[2]].data()) ^
    *reinterpret_cast<uint32_t *>(table[3][data[3]].data()) ^
    *reinterpret_cast<const uint32_t *>(key[0].data()),
    *reinterpret_cast<uint32_t *>(table[4][data[4]].data()) ^
    *reinterpret_cast<uint32_t *>(table[5][data[5]].data()) ^
    *reinterpret_cast<uint32_t *>(table[6][data[6]].data()) ^
    *reinterpret_cast<uint32_t *>(table[7][data[7]].data()) ^
    *reinterpret_cast<const uint32_t *>(key[1].data()),
    *reinterpret_cast<uint32_t *>(table[8][data[8]].data()) ^
    *reinterpret_cast<uint32_t *>(table[9][data[9]].data()) ^
    *reinterpret_cast<uint32_t *>(table[10][data[10]].data()) ^
    *reinterpret_cast<uint32_t *>(table[11][data[11]].data()) ^
    *reinterpret_cast<const uint32_t *>(key[2].data()),
    *reinterpret_cast<uint32_t *>(table[12][data[12]].data()) ^
    *reinterpret_cast<uint32_t *>(table[13][data[13]].data()) ^
    *reinterpret_cast<uint32_t *>(table[14][data[14]].data()) ^
    *reinterpret_cast<uint32_t *>(table[15][data[15]].data()) ^
    *reinterpret_cast<const uint32_t *>(key[3].data())
  };
  memcpy(data.data(), temp.data(), 16);
}

// TFIT_op_iCMAC, TFIT_wbaes_cbc_encrypt_iAES3, TFIT_wbaes_cbc_decrypt_iAES4
// mac: AES-128, enc/dec: AES-256
//   TFIT_rInv[16]
//     TFIT_rInv_0_3_0_iAES3
//     TFIT_rInv_3_3_0_iAES3
//   TFIT_r[16]
//     TFIT_r_0_3_0_iAES3
//     TFIT_r_3_3_0_iAES3
//   TFIT_rij[20 unique + (mac: 124, enc/dec: 188)] // Rijndael? 9/13 rounds?
//     TFIT_rij_0_15_iAES3
//     TFIT_rij_1_03_iAES3
//   TFIT_sInv[16] // 10th/14th round?
//     TFIT_sInv_0_3_0_iAES3
//     TFIT_sInv_3_3_0_iAES3
//   TFIT_s[16]
//     TFIT_s_0_3_0_iAES3
//     TFIT_s_3_3_0_iAES3
template<size_t Rounds>
void TFIT<Rounds>::ProcessBlock(std::span<uint8_t, 16> src, std::array<uint8_t, 16> &dst) {
  std::array<uint8_t, 16> temp;
  std::copy(std::begin(src), std::end(src), std::begin(temp));

  RoundA(temp, keys_[0], tables_[0]); // rInv
  RoundA(temp, keys_[1], tables_[1]); // r
  for (size_t i = 2; i < Rounds - 1; i++) {
    RoundB(temp, keys_[i], tables_[i]); // rij, sInv
  }
  RoundA(temp, keys_[Rounds - 1], tables_[Rounds - 1]); // s

  dst = temp;
}

template class TFIT<17>;
template class TFIT<13>;
