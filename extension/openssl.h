#include <openssl/ssl.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
//#include <openssl/ec.h>
//#include <openssl/ossl_typ.h>
#include <openssl/err.h>

#ifndef OPENSSL_VERSION_NUMBER
#define OPENSSL_VERSION_NUMBER 0x1010001fL
#endif

#define OPENSSL_RECV_WAIT	(-2147483647) //S2GM

#define MYSSLWORK_OK			1
#define MYSSLWORK_WANT_READ		SSL_ERROR_WANT_READ
#define MYSSLWORK_WANT_WRITE	SSL_ERROR_WANT_WRITE

class MySSL{
	SSL *ssl;
	SSL_CTX *ctx;
	int work;

public:
	MySSL();

	int IsWork();
	int Connect(SOCKET sock);
	int AcceptFile(SOCKET sock, VString cert, VString key, int typefile = 0);
	int Accept(SOCKET sock, VString cert, VString key, int typefile = 0);
	int LoadCert(SSL_CTX *ctx, VString cert, VString key, int typefile = 0);
	int LoadCertFile(SSL_CTX *ctx, VString cert, VString key);
	int Recv(void *buf, int sz);
	int Send(const char *buf, int sz);
	int Close();
	void Release();
	~MySSL();
};


// RSA
void RsaCreateKeys(TString &pub, TString &sec);
int RsaSaveCreateKeys(VString pubf, VString secf);
TString RsaOperation(int type, VString key, VString text);
TString RsaPublicEncode(VString key, VString text);
TString RsaPublicDecode(VString key, VString text);
TString RSAPrivateEncode(VString key, VString text);
TString RsaPrivateDecode(VString key, VString text);

// AES
TString AesEncode(VString line, VString ckey, VString ivec);
TString AesDecode(VString line, VString ckey, VString ivec);
TString AesEncodeMyDataBlock(VString data, VString ckey, VString ivek);
TString AESDecodeMyDataBlock(VString data, VString ckey, VString ivek);
TString TEncodeData(VString data);
TString TDecodeData(VString data);

// Uniq
TString SslGetUniqLine();
TString RandGetUniqLine();

// Secure
int SslProgSecureInit();

// Other
int OpenSSLCreateRSAPrivateKey(TString &r_key, int bits = 2048);
int OpenSSLCreateCertificateRequest(VString r_key, TString &r_req, VString ca, VString co, VString cn);
int OpenSSLCreateCACertificate(VString r_key, TString &r_req, VString ca, VString co, VString cn);
bool OpenSSLSignCertificateLoadCA(X509 ** px509, VString ca_cert);
bool OpenSSLSignCertificateLoadCAPrivateKey(EVP_PKEY **ppkey, VString ca_key);
bool OpenSSLSignCertificateLoadX509Req(X509_REQ **ppReq, VString req);
int OpenSSLSignCertificateDoX509Sign(X509 *cert, EVP_PKEY *pkey, const EVP_MD *md);
int OpenSSLSignCertificate(TString &r_cert, VString ca_cert, VString ca_key, VString r_req, int serial);
