#include "cryption.h"

QByteArray Cryption::encryptMessage(
        const SecByteBlock& key,
        const byte* iv,
        int ivByteLen,
        const QByteArray& message)
{
    GCM<AES>::Encryption e;
    e.SetKeyWithIV(key, key.size(), iv, ivByteLen);

    std::string cipher;
    AuthenticatedEncryptionFilter ef( e,
        new StringSink(cipher), false,
        TAG_SIZE /* MAC_AT_END */
    );

    ef.ChannelPut(AAD_CHANNEL, iv, ivByteLen);
    ef.ChannelMessageEnd(AAD_CHANNEL);

    ef.ChannelPut(DEFAULT_CHANNEL, (byte*)message.constData(),
                  message.size());
    ef.ChannelMessageEnd(DEFAULT_CHANNEL);


    return QByteArray(cipher.c_str(), cipher.length());
}

QPair<QByteArray, QByteArray> Cryption::encryptMessage(
        const SecByteBlock& key,
        const QByteArray& message)
{
    AutoSeededRandomPool rng;
    byte iv[12];
    rng.GenerateBlock(iv, sizeof(iv));
    return qMakePair(QByteArray((char*)iv, sizeof(iv)),
                     encryptMessage(key, iv, sizeof(iv), message)
                     );
}


QPair<bool, QByteArray> Cryption::decryptMessage(
        const SecByteBlock& key,
        const QPair<QByteArray, QByteArray>& message)
{
    QByteArray sendIV = message.first;
    QByteArray sendCipher = message.second;
    bool valid = true;

    GCM<AES>::Decryption d;
    d.SetKeyWithIV( key, key.size(), (byte*)sendIV.data(), sendIV.size() );

    std::string cipher(sendCipher.constData(), sendCipher.size());
    std::string enc = cipher.substr( 0, cipher.length()-TAG_SIZE );
    std::string mac = cipher.substr( cipher.length()-TAG_SIZE );
    std::string retrieved;

    try
    {
        AuthenticatedDecryptionFilter df( d, NULL,
            AuthenticatedDecryptionFilter::MAC_AT_BEGIN |
            AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE );


        df.ChannelPut( DEFAULT_CHANNEL, (const byte*)mac.data(), mac.size() );
        df.ChannelPut( AAD_CHANNEL, (const byte*)sendIV.data(), sendIV.size() );
        df.ChannelPut( DEFAULT_CHANNEL,(const byte*) enc.data(), enc.size() );

        df.ChannelMessageEnd( AAD_CHANNEL );
        df.ChannelMessageEnd( DEFAULT_CHANNEL );

        bool b = false;
        b = df.GetLastResult();
        if (!b)
            throw 0;

        size_t n = (size_t)-1;
        df.SetRetrievalChannel( DEFAULT_CHANNEL );
        n = (size_t)df.MaxRetrievable();
        retrieved.resize( n );

        if( n > 0 ) { df.Get( (byte*)retrieved.data(), n ); }

    }
    catch(...)
    {
        valid = false;
    }
    return qMakePair(valid,
                     QByteArray(retrieved.data(), retrieved.size())
                     );
}


SecByteBlock Cryption::getHashFromPassword(const QString& password)
{
    SHA256 hash;
    byte aes_key[SHA256::DIGESTSIZE];
    memset(aes_key, 0, SHA256::DIGESTSIZE);
    QByteArray password_bytes(password.toUtf8());
    hash.CalculateDigest(aes_key, (const byte*)password_bytes.constData(),
                         password_bytes.size());
    SecByteBlock key(aes_key, SHA256::DIGESTSIZE);
    memset(aes_key, 0, SHA256::DIGESTSIZE);
    password_bytes.fill(0);
    return key;
}


void Cryption::generateClientRSAPiar(
        const QString& skpath,
        const QString& pkpath,
        const QString& password,
        int RSAKeyLength)
{
    //take SHA256 from password (256 bits = 32 bytes  - max AES)
    //key length
    SecByteBlock key = getHashFromPassword(password);

    //generate sk and pk
    AutoSeededRandomPool rng;
    RSA::PrivateKey privateKey;
    privateKey.Initialize(rng, RSAKeyLength);
    RSA::PublicKey publicKey(privateKey);

    //saving pk
    Base64Encoder publicsink(new FileSink(pkpath.toUtf8().constData()));
    publicKey.DEREncode(publicsink);
    publicsink.MessageEnd();

    std::string res;
    StringSink ss(res);
    privateKey.Save(ss);

    // Generate a random IV
    byte iv[AES::BLOCKSIZE];
    rng.GenerateBlock(iv, AES::BLOCKSIZE);

    //encrypt sk with SHA256(pw)
    CTR_Mode<AES>::Encryption cfbEncryption(key, key.size(),
                                            iv, sizeof(iv));

    QFile fout(skpath);
    fout.open(QIODevice::WriteOnly);
    QDataStream sout(&fout);
    sout.writeRawData((char*)iv, AES::BLOCKSIZE);

    std::string crypted;
    StreamTransformationFilter stf(cfbEncryption,
                                   new StringSink(crypted));
    stf.Put((byte*)res.data(), res.size());
    stf.MessageEnd();

    sout.writeRawData(crypted.data(), crypted.size());

    fout.close();
}

QByteArray Cryption::checkClientSK(
        const QString& filename,
        const QString& password,
        const QByteArray& cipher)
{
    SecByteBlock key = getHashFromPassword(password);

    //read iv and crypted key
    string file_data;
    FileSource(filename.toUtf8().constData(), true,
               new StringSink(file_data));

    string iv = file_data.substr(0, AES::BLOCKSIZE);
    file_data.erase(0, AES::BLOCKSIZE);

    CTR_Mode<AES>::Decryption decryptor(key, key.size(),
                                        (const byte*)iv.data(), iv.size());

    std::string sk;
    StreamTransformationFilter stf(decryptor, new StringSink(sk));
    stf.Put((byte*)file_data.c_str(), file_data.size());
    stf.MessageEnd();

    string recovered;
    try
    {
        RSA::PrivateKey privateKey;
        StringSource src(sk, true);
        privateKey.BERDecode(src);

        sk.erase(0, string::npos);

        RSAES_OAEP_SHA_Decryptor d(privateKey);

        AutoSeededRandomPool rng;

        std::string str_cipher(cipher.constData(), cipher.size());
        StringSource(str_cipher, true,
            new PK_DecryptorFilter(rng, d,
                new StringSink(recovered)
           )
        );
    }
    catch (...)
    {
        return QByteArray();
    }

    return QByteArray(recovered.data(), recovered.size());
}

QPair<QByteArray, QByteArray> Cryption::generateTextForChecking(
        const QByteArray& clientPK,
        int textByteLength)
{
    StringSource ss((const byte*)clientPK.constData(), clientPK.size(),
                    true, new Base64Decoder);
    RSA::PublicKey pubKey;
    pubKey.BERDecode(ss);

    AutoSeededRandomPool rng;
    SecByteBlock randomText(textByteLength);
    rng.GenerateBlock(randomText, randomText.size());

    RSAES_OAEP_SHA_Encryptor e(pubKey);
    string cipher;
    StringSource(randomText, randomText.size(), true,
                 new PK_EncryptorFilter(rng, e,
                                        new StringSink(cipher)
                                        )
                 );

    return qMakePair(QByteArray((const char*)randomText.data(), randomText.size()),
                     QByteArray(cipher.data(), cipher.size()));
}
