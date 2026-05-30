#ifndef CRYPTO_H
#define CRYPTO_H
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include "meander.h"

// ─────────────────────────────────────────────────────────────────────────
//  Криптографическое ядро МеандрШифр
//
//  1. Из последовательности обхода меандра выводится 256-битный ключ (SHA-256).
//  2. Ключ расширяется хэш-цепочкой до длины сообщения.
//  3. Применяется потоковое XOR-шифрование (операция симметрична).
// ─────────────────────────────────────────────────────────────────────────

using Bytes = std::vector<uint8_t>;

// SHA-256 от произвольного буфера → 32 байта.
std::array<uint8_t, 32> sha256(const Bytes& data);

// Выводит 256-битный ключ из последовательности обхода меандра.
std::array<uint8_t, 32> deriveKey(const Meander& m);

// Расширяет ключ хэш-цепочкой до length байт: H(key||0), H(key||1), …
Bytes expandKey(const std::array<uint8_t, 32>& key, std::size_t length);

// XOR-шифрование/дешифрование (одна и та же функция в обе стороны).
Bytes xorCipher(const Bytes& data, const std::array<uint8_t, 32>& key);

// Энтропия Шеннона буфера (бит на байт).
double shannonEntropy(const Bytes& data);

// Перевод буфера в hex-строку и обратно.
std::string toHex(const Bytes& data);
bool fromHex(const std::string& hex, Bytes& out);

#endif // CRYPTO_H
