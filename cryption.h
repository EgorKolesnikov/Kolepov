#ifndef CRYPTION_H
#define CRYPTION_H

#include <QtCore>

#include <string>
using std::string;

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "secblock.h"
using CryptoPP::SecByteBlock;

#include "aes.h"
using CryptoPP::AES;

#include "rsa.h"
using CryptoPP::RSA;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;

#include "base64.h"
using CryptoPP::Base64Encoder;
using CryptoPP::Base64Decoder;

#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

#include "gcm.h"
using CryptoPP::GCM;
using CryptoPP::AAD_CHANNEL;
using CryptoPP::DEFAULT_CHANNEL;

#include "cryptlib.h"
using CryptoPP::BufferedTransformation;
using CryptoPP::AuthenticatedSymmetricCipher;

#include "files.h"
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include "modes.h"
using CryptoPP::CTR_Mode;

#include "sha.h"
using CryptoPP::SHA256;


class Cryption
{
public:

    //size of the MAC
    const static int TAG_SIZE = 16;

    //Authenticated encryption
    //returns (iv, cipher)
    static QPair<QByteArray, QByteArray> encryptMessage(
            const SecByteBlock& key,
            const QByteArray& message);

    //Authenticated decryption
    //returns (isAuthenticated, plain text)
    static QPair<bool, QByteArray> decryptMessage(
            const SecByteBlock& key,
            const QPair<QByteArray, QByteArray>& message);

    //Generate and save to files RSA key pair for client. Client's PK is
    //encoded (not encrypted) with Base64 and must be placed to
    //the Users table is the Server.
    //Client's SK is encrypted with the password and must be stored in his
    //computer.
    static void generateClientRSAPiar(
            const QString& skpath,
            const QString& pkpath,
            const QString& password,
            int RSAKeyLength);

    //Take Client's SK from the file, decrypt it with password and then
    //decrpyt cipher with a Client's SK
    static QByteArray checkClientSK(
            const QString& filename,
            const QString& password,
            const QByteArray& cipher);

    //Generate a random text and encrypts with client's PK. Client's PK
    //is in Base64.
    //return (original text, encrypted text)
    static QPair<QByteArray, QByteArray> generateTextForChecking(
            const QByteArray& clientPK,
            int textByteLength);

private:

    //Used by generateClientRSAPiar() and checkClientSK()
    //returns SHA256 hash of the password
    static SecByteBlock getHashFromPassword(const QString& password);

};

#endif // CRYPTION_H
