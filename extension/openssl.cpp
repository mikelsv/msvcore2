#if OPENSSL_VERSION_NUMBER >= 0x1010000fL
#if _MSC_VER == 1900
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\VS\\2015\\libssl.lib")
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\VS\\2015\\libcrypto.lib") 
#else
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\libssl.lib")
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\libcrypto.lib") 
	#pragma comment (lib, "crypt32")
#endif
#else
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\ssleay32.lib") 
	#pragma comment(lib,"C:\\Program Files (x86)\\OpenSSL\\lib\\libeay32.lib") 
#endif

// Compile perl Configure VC-WIN32 no-shared
// nmake
// nmake test
// nmake install

// Linker: /DYNAMICBASE crypt32.lib 

// ZLIB: #include "../../opensource/msvcore/crossplatform-zlib.cpp"

// include path C:\Program Files (x86)\OpenSSL\include
// include path ..\..\openssl-1.1.0a\include

// http://developer.covenanteyes.com/building-openssl-for-visual-studio/
// http://phpseclib.sourceforge.net/
// http://stackoverflow.com/questions/4484246/encrypt-and-decrypt-text-with-rsa-in-php


// See MsspCore class
#ifdef USEMSV_OPENSSL_DEBUG

// MSV Smart Ssl Pointer for test(memory leak)
class MsspCoreEl{
public:
	void *p;
};

class MsspCore{
	OList<MsspCoreEl> arr;

public:
	void New(void *v){
		if(!v)
			return ;

		MsspCoreEl *p = arr.NewE();
		p->p = v;

		return ;
	}

	void Free(void *v){
		if(!v)
			return ;

		MsspCoreEl *p = 0;
		while(p = arr.Next(p)){
			if(p->p == v){
				arr.Free(p);
				return ;
			}
		}

		WarningReFree();

		return ;
	}

	void End(){
		MsspCoreEl *p = arr.Next(0);
		if(p)
			WarningMemoryLeak();
		return ;
	}

	~MsspCore(){
		MsspCoreEl *p = arr.Next(0);
		if(p)
			WarningMemoryLeak2();
		return ;
	}

	void WarningMemoryLeak(){
		print("WARNING! Mssp memory leak found!\r\n");
	}

	void WarningMemoryLeak2(){
		print("WARNING! Mssp free error!\r\n");
	}

	void WarningReFree(){
		print("WARNING! Mssp free again!\r\n");
	}

};

	#define MsspInit() MsspCore _mssp_core;
	#define MsspNew(p) _mssp_core.New(p);
	#define MsspFree(p) _mssp_core.Free(p);
	#define MsspEnd() _mssp_core.End();
#else
	#define MsspInit()
	#define MsspNew(p)
	#define MsspFree(p)
	#define MsspEnd()
#endif


class MsvCoreActivateSsl{
public:
	MsvCoreActivateSsl(){
		SSL_library_init();
		SSL_load_error_strings();		/* load all error messages */
		OpenSSL_add_all_algorithms();	/* load & register cryptos */
		return ;
	}

	~MsvCoreActivateSsl(){
		ERR_free_strings();
		EVP_cleanup();
		ERR_remove_state(0);
		//sk_SSL_COMP_free(SSL_COMP_get_compression_methods());

		//sk_free(SSL_COMP_get_compression_methods());
		//SSL_COMP_free_compression_methods();
	}
} MsvCoreActivateSsl;


// class MySSL
MySSL::MySSL(){
	ssl = 0;
	ctx = 0;
	work = 0;
}

int MySSL::IsWork(){
	return work >= 1;
}

int MySSL::Connect(SOCKET sock){
	MsspInit();
	Release();

	ctx = SSL_CTX_new(SSLv23_method());
	MsspNew(ctx);

	if(!ctx){
		MsspEnd();
		return 0;
	}

	ssl = SSL_new(ctx);
	MsspNew(ssl);

	if(!ssl){
		Release();
		MsspEnd();
		return 0;
	}

	if(!SSL_set_fd(ssl, sock)){
		Release();
		MsspEnd();
		return 0;
	}

	SSL_CTX_set_timeout(ctx, 60);
		
	int eid;

	if((eid = SSL_connect(ssl)) < 0){
		int sid = SSL_get_error(ssl, eid);
		if(sid == SSL_ERROR_WANT_READ || sid == SSL_ERROR_WANT_WRITE)
			work = MYSSLWORK_WANT_WRITE;
		else
			work = 0;
	}
	else
		work = eid >= 0;

	MsspFree(ctx);
	MsspFree(ssl);
	MsspEnd();
	return work;
}

int MySSL::AcceptFile(SOCKET sock, VString cert, VString key, int typefile){
	return Accept(sock, cert, key, 1);
}

int MySSL::Accept(SOCKET sock, VString cert, VString key, int typefile){
	MsspInit();
	Release();

	ctx = SSL_CTX_new(SSLv23_server_method());
	MsspNew(ctx);

	if(!ctx){
		MsspEnd();
		Release();
		return 0;
	}

	if(!typefile && !LoadCert(ctx, cert, key) || typefile && !LoadCertFile(ctx, cert, key)){
		MsspFree(ctx);
		MsspEnd();
		Release();
		return 0;
	}

	ssl = SSL_new(ctx);
	MsspNew(ssl);

	if(!ssl || !SSL_set_fd(ssl, sock) || !SSL_CTX_set_timeout(ctx, 60)){
		MsspFree(ctx);
		MsspFree(ssl);
		MsspEnd();
		Release();
		return 0;
	}

	int eid;

	if((eid = SSL_accept(ssl)) < 0){
		int sid = SSL_get_error(ssl, eid);
			
		if(sid == SSL_ERROR_WANT_READ){
			work = MYSSLWORK_WANT_READ;
		}else {
			char buf[S8K];
			ERR_print_errors_fp(stderr);
			ERR_print_errors_fp(stdout);
			//ERR_error_string_n(sid, buf, sizeof(buf));
			//char * err = ERR_error_string(eid, buf);
				
			MsspFree(ctx);
			MsspFree(ssl);
			MsspEnd();
			Release();
			return 0;
		}
	}
	else 
		work = 1;
		
	MsspFree(ctx);
	MsspFree(ssl);
	MsspEnd();
	return 1;
}

//char buf[8*1024];
//    char reply[8*1024];
//    int sd, bytes;
//    const char* HTMLecho="<html><body><pre>%s</pre></body></html>\n\n";
//
	//sd = SSL_get_fd(ssl);
	//SSL_accept(ssl);

	//bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
	//
	//memcpy(buf, "HTTP/1.1 200 OK\r\n\r\n", 19);
	//bytes = SSL_write(sd, buf, 19); /* get request */

    //if ( SSL_accept(ssl) == -1 ){     /* do SSL-protocol accept */
	//	int err = ERR_get_error();
    //    ERR_print_errors_fp(stderr);
	//}
 //   else
 //   {
 //       //ShowCerts(ssl);        /* get any certificates */
 //       bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
 //       if ( bytes > 0 )
 //       {
 //           buf[bytes] = 0;
 //           printf("Client msg: \"%s\"\n", buf);
 //           sprintf(reply, HTMLecho, buf);   /* construct reply */
 //           SSL_write(ssl, reply, strlen(reply)); /* send reply */
 //       }
 //       else
 //           ERR_print_errors_fp(stderr);
 //   }
		// work = 1;


int MySSL::LoadCert(SSL_CTX *ctx, VString cert, VString key, int typefile){
	if(!cert || !key)
		return 0;

	int ret;

	// Read X509
	RSA *rsa = 0;
	X509 *x509 = NULL;
	BIO *b = BIO_new_mem_buf(cert, cert);
	x509 = PEM_read_bio_X509(b, NULL, 0, NULL);

	ret = SSL_CTX_use_certificate(ctx, x509);

	BIO_free(b);
	X509_free(x509);
		
	if(ret <= 0){
		ERR_print_errors_fp(stderr);
		return 0;
	}

	// Read Private key
	b = BIO_new_mem_buf(key, key);
	rsa = PEM_read_bio_RSAPrivateKey(b, NULL, 0, NULL);

	ret = SSL_CTX_use_RSAPrivateKey(ctx, rsa);

	BIO_free(b);
	RSA_free(rsa);

	if(ret <= 0){
		ERR_print_errors_fp(stderr);
		return 0;
	}

	/* verify private key */
	if(!SSL_CTX_check_private_key(ctx)){
		fprintf(stderr, "Private key does not match the public certificate\n");
		return 0;
	}

	//New lines - Force the client-side have a certificate
	//SSL_CTX_set_cipher_list(ctx, "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:SRP-DSS-AES-256-CBC-SHA:SRP-RSA-AES-256-CBC-SHA:DHE-DSS-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA256:DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:DHE-RSA-CAMELLIA256-SHA:DHE-DSS-CAMELLIA256-SHA:ECDH-RSA-AES256-GCM-SHA384:ECDH-ECDSA-AES256-GCM-SHA384:ECDH-RSA-AES256-SHA384:ECDH-ECDSA-AES256-SHA384:ECDH-RSA-AES256-SHA:ECDH-ECDSA-AES256-SHA:AES256-GCM-SHA384:AES256-SHA256:AES256-SHA:CAMELLIA256-SHA:PSK-AES256-CBC-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-DES-CBC3-SHA:SRP-DSS-3DES-EDE-CBC-SHA:SRP-RSA-3DES-EDE-CBC-SHA:EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:ECDH-RSA-DES-CBC3-SHA:ECDH-ECDSA-DES-CBC3-SHA:DES-CBC3-SHA:PSK-3DES-EDE-CBC-SHA:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:SRP-DSS-AES-128-CBC-SHA:SRP-RSA-AES-128-CBC-SHA:DHE-DSS-AES128-GCM-SHA256:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES128-SHA256:DHE-DSS-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:DHE-RSA-SEED-SHA:DHE-DSS-SEED-SHA:DHE-RSA-CAMELLIA128-SHA:DHE-DSS-CAMELLIA128-SHA:ECDH-RSA-AES128-GCM-SHA256:ECDH-ECDSA-AES128-GCM-SHA256:ECDH-RSA-AES128-SHA256:ECDH-ECDSA-AES128-SHA256:ECDH-RSA-AES128-SHA:ECDH-ECDSA-AES128-SHA:AES128-GCM-SHA256:AES128-SHA256:AES128-SHA:RC4-SHA:SEED-SHA:CAMELLIA128-SHA:IDEA-CBC-SHA:PSK-AES128-CBC-SHA:ECDHE-RSA-RC4-SHA:ECDHE-ECDSA-RC4-SHA:ECDH-RSA-RC4-SHA:ECDH-ECDSA-RC4-SHA:RC4-MD5:PSK-RC4-SHA:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC4-MD5"); 
	//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	//SSL_CTX_set_verify_depth(ctx, 4);
	//End new lines

	return 1;
}

int MySSL::LoadCertFile(SSL_CTX *ctx, VString cert, VString key){
	TString CertFile = cert, KeyFile = key;

	//New lines 
	if(SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile) != 1){
		ERR_print_errors_fp(stderr);
		return 0;
	}

	if (SSL_CTX_set_default_verify_paths(ctx) != 1){
		ERR_print_errors_fp(stderr);
		return 0;
	}
	//End new lines

	/* set the local certificate from CertFile */
	if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		return 0;
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		return 0;
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(ctx)){
		fprintf(stderr, "Private key does not match the public certificate\n");
		return 0;
	}

	//New lines - Force the client-side have a certificate
	//SSL_CTX_set_cipher_list(ctx, "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:SRP-DSS-AES-256-CBC-SHA:SRP-RSA-AES-256-CBC-SHA:DHE-DSS-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA256:DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:DHE-RSA-CAMELLIA256-SHA:DHE-DSS-CAMELLIA256-SHA:ECDH-RSA-AES256-GCM-SHA384:ECDH-ECDSA-AES256-GCM-SHA384:ECDH-RSA-AES256-SHA384:ECDH-ECDSA-AES256-SHA384:ECDH-RSA-AES256-SHA:ECDH-ECDSA-AES256-SHA:AES256-GCM-SHA384:AES256-SHA256:AES256-SHA:CAMELLIA256-SHA:PSK-AES256-CBC-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-DES-CBC3-SHA:SRP-DSS-3DES-EDE-CBC-SHA:SRP-RSA-3DES-EDE-CBC-SHA:EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:ECDH-RSA-DES-CBC3-SHA:ECDH-ECDSA-DES-CBC3-SHA:DES-CBC3-SHA:PSK-3DES-EDE-CBC-SHA:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:SRP-DSS-AES-128-CBC-SHA:SRP-RSA-AES-128-CBC-SHA:DHE-DSS-AES128-GCM-SHA256:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES128-SHA256:DHE-DSS-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:DHE-RSA-SEED-SHA:DHE-DSS-SEED-SHA:DHE-RSA-CAMELLIA128-SHA:DHE-DSS-CAMELLIA128-SHA:ECDH-RSA-AES128-GCM-SHA256:ECDH-ECDSA-AES128-GCM-SHA256:ECDH-RSA-AES128-SHA256:ECDH-ECDSA-AES128-SHA256:ECDH-RSA-AES128-SHA:ECDH-ECDSA-AES128-SHA:AES128-GCM-SHA256:AES128-SHA256:AES128-SHA:RC4-SHA:SEED-SHA:CAMELLIA128-SHA:IDEA-CBC-SHA:PSK-AES128-CBC-SHA:ECDHE-RSA-RC4-SHA:ECDHE-ECDSA-RC4-SHA:ECDH-RSA-RC4-SHA:ECDH-ECDSA-RC4-SHA:RC4-MD5:PSK-RC4-SHA:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC4-MD5"); 
	//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	//SSL_CTX_set_verify_depth(ctx, 4);
	//End new lines

	return 1;
}

int MySSL::Recv(void *buf, int sz){
	if(!work || !ssl)
		return 0;

	if(work == MYSSLWORK_WANT_READ){
		int eid = SSL_accept(ssl);
		if(eid == -1){
			int sid = SSL_get_error(ssl, eid);
			if(sid == SSL_ERROR_WANT_READ)
				return OPENSSL_RECV_WAIT;
			return -1;
		}
		work = MYSSLWORK_OK;
		return OPENSSL_RECV_WAIT;
	}

	if(work == MYSSLWORK_WANT_WRITE){
		int eid = SSL_connect(ssl);
		if(eid == -1){
			int sid = SSL_get_error(ssl, eid);
			if(sid == SSL_ERROR_WANT_READ)
				return OPENSSL_RECV_WAIT;
			return -1;
		}
		work = MYSSLWORK_OK;
	}

	int ret = SSL_read(ssl, buf, sz), sret;
	if(ret == -1){
		sret = SSL_get_error(ssl, ret);
		if(sret == SSL_ERROR_WANT_READ)
			ret = OPENSSL_RECV_WAIT;
	}

	return ret;
}

int MySSL::Send(const char *buf, int sz){
	if(!work || !ssl)
		return 0;

	if(work == MYSSLWORK_WANT_WRITE){// return OPENSSL_RECV_WAIT;
		int eid = SSL_connect(ssl);
		if(eid == -1){
			int sid = SSL_get_error(ssl, eid);
			if(sid == SSL_ERROR_WANT_READ)
				return OPENSSL_RECV_WAIT;
			return 0;
		}
		work = MYSSLWORK_OK;
		return OPENSSL_RECV_WAIT;
	}

	int ret = SSL_write(ssl, buf, sz);
	if(ret == -1){
		int sret = SSL_get_error(ssl, ret);
		if(sret == SSL_ERROR_WANT_WRITE)
			ret = OPENSSL_RECV_WAIT;
	}

	return ret;
}

int MySSL::Close(){
	if(!work)
		return 0;

	work = 0;

	if(ssl){
		SSL_shutdown(ssl);
	}

	return 1;
}

void MySSL::Release(){
	if(work){
		work = 0;

		if(ssl)
			SSL_shutdown(ssl);
	}

	if(ssl)
		SSL_free(ssl);

	if(ctx)
		SSL_CTX_free(ctx);

	ssl = 0;
	ctx = 0;
	work = 0;
}

MySSL::~MySSL(){
	Release();
}
// End class MySSL


void RsaCreateKeys(TString &pub, TString &sec){
	// generate keys
	RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
	int sz;

	BIO *bp = BIO_new(BIO_s_mem());
	BIO *bs = BIO_new(BIO_s_mem());

	PEM_write_bio_RSA_PUBKEY(bp, rsa);
	PEM_write_bio_RSAPrivateKey(bs, rsa, 0, 0, 0, 0, 0);

	// pub
	sz = BIO_ctrl_pending(bp);
	pub.Reserve(sz);
	BIO_read(bp, pub, sz);

	// sec
	sz = BIO_ctrl_pending(bs);
	sec.Reserve(sz);
	BIO_read(bs, sec, sz);

	BIO_free(bp);
	BIO_free(bs);
	RSA_free(rsa);

	return ;
}

int RsaSaveCreateKeys(VString pubf, VString secf){
	TString pub, sec;
	int ret = 1;

	RsaCreateKeys(pub, sec);
	ret &= SaveFile(pubf, pub) > 0;
	ret &= SaveFile(secf, sec) > 0;
	return ret;
}

TString RsaOperation(int type, VString key, VString text){
	if(!text)
		return TString();

	TString ret;
	RSA *rsa = RSA_new(), *r;
	BIO *b = BIO_new_mem_buf(key, key);
	int sz = 0;

	if(type < 2)
		r = PEM_read_bio_RSA_PUBKEY(b, &rsa, 0, 0);
	else
		r = PEM_read_bio_RSAPrivateKey(b, &rsa, 0, 0);

	if(!r){
		BIO_free(b);
		RSA_free(rsa);
		return TString();
	}
	
	switch(type){
	case 0:
		//r=PEM_read_bio_RSA_PUBKEY(b, &rsa, 0, 0);
		//if(!r)
		//	return MString();
		ret.Reserve(RSA_size(rsa));
		sz = RSA_public_encrypt(text, text, ret, rsa, RSA_PKCS1_PADDING);
	break;
	case 1:
		//PEM_read_bio_RSA_PUBKEY(b, &rsa, 0, 0);
		ret.Reserve(RSA_size(rsa));
		sz = RSA_public_decrypt(text, text, ret, rsa, RSA_PKCS1_PADDING);
	break;
	case 2:
		//PEM_read_bio_RSAPrivateKey(b, &rsa, 0, 0);
		ret.Reserve(RSA_size(rsa));
		sz = RSA_private_encrypt(text, text, ret, rsa, RSA_PKCS1_PADDING);
	break;
	case 3:
		//PEM_read_bio_RSAPrivateKey(b, &rsa, 0, 0);
		ret.Reserve(RSA_size(rsa));
		sz = RSA_private_decrypt(text, text, ret, rsa, RSA_PKCS1_PADDING);
	break;
	}
	
	BIO_free(b);
	RSA_free(rsa);

	if(sz == -1){
		int err = ERR_get_error();
		ERR_error_string(ret, ret);
	}

	ret.Reserve(sz);
	return ret;
}

TString RsaPublicEncode(VString key, VString text){
	return RsaOperation(0, key, text);
}

TString RsaPublicDecode(VString key, VString text){
	return RsaOperation(1, key, text);
}

TString RSAPrivateEncode(VString key, VString text){
	return RsaOperation(2, key, text);
}

TString RsaPrivateDecode(VString key, VString text){
	return RsaOperation(3, key, text);
}

// AES
TString AesEncode(VString line, VString ckey, VString ivec){
	TString ret;
	ret.Reserve(line.sz*2);
	int retlen = 0, retlenf = 0;

	if(!ckey || !ivec)
		return TString();

	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();
	//EVP_CIPHER_CTX_init(ctx);

	EVP_EncryptInit(ctx, EVP_aes_128_cbc(), ckey, ivec);
	EVP_EncryptUpdate(ctx, ret, &retlen, line, line);
	EVP_EncryptFinal(ctx, ret.data+retlen, &retlenf);

	EVP_CIPHER_CTX_free(ctx);

	ret.Reserve(retlen+retlenf);
	return ret;
}

TString AesDecode(VString line, VString ckey, VString ivec){
	TString ret;
	ret.Reserve(line.sz+16);
	int retlen = 0, retlenf = 0;

	if(!ckey || !ivec)
		return TString();

	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();

	EVP_DecryptInit(ctx, EVP_aes_128_cbc(), ckey, ivec);
	EVP_DecryptUpdate(ctx, ret, &retlen, line, line);
	EVP_DecryptFinal(ctx, ret.data+retlen, &retlenf);
	
	//EVP_CIPHER_CTX_cleanup(&ctx);
	EVP_CIPHER_CTX_free(ctx);

	ret.Reserve(retlen + retlenf);
	return ret;
}

TString SslGetUniqLine(){
	TString pub, sec;
	RsaCreateKeys(pub, sec);
	return RsaPublicEncode(pub, sec.str(32, 100));
}

TString RandGetUniqLine(){
	TString ret;
	ret.Reserve(256);

	unsigned char *r = ret, *to = ret.endu();
	srand(time());

	while(r < to){
		*r++ = rand() % 256;
	}

	return ret;
}

// prog secure
VString _ssl_init_p="-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA8At66rwd96Ra2eNvKp9KMBW0Ex6Q8lwHep+/D8lWhuwSeU0+CSm9daQS0Te89Gcdo1x5XfPoe77tD0fvrfxGinJumAAVILRa8XME/Ml1BHRTq/cVqDoMWOkax8mYGBHwzW815DqmDqe9xH7LVGGbWzz4JY+ji2vYt2dZk7HZBY+2h3aJIoRB/RrHYdb6USVt325RMDpGsUONXpx3Ep7vnBuWEcTqY45mnDLKPpl64r75j9NXOVe0KlKUMHzPyns1nOYIJjzNAgnxH3jGle11sK535/yt81h7uh1p7ozFNHj88LN0q19ebrwQU2qBQmb/LJL+Z/IvsPDi4yVGTcj3EwIDAQAB-----END PUBLIC KEY-----";
VString _ssl_init_s="-----BEGIN RSA PRIVATE KEY-----MIIEowIBAAKCAQEA8At66rwd96Ra2eNvKp9KMBW0Ex6Q8lwHep+/D8lWhuwSeU0+CSm9daQS0Te89Gcdo1x5XfPoe77tD0fvrfxGinJumAAVILRa8XME/Ml1BHRTq/cVqDoMWOkax8mYGBHwzW815DqmDqe9xH7LVGGbWzz4JY+ji2vYt2dZk7HZBY+2h3aJIoRB/RrHYdb6USVt325RMDpGsUONXpx3Ep7vnBuWEcTqY45mnDLKPpl64r75j9NXOVe0KlKUMHzPyns1nOYIJjzNAgnxH3jGle11sK535/yt81h7uh1p7ozFNHj88LN0q19ebrwQU2qBQmb/LJL+Z/IvsPDi4yVGTcj3EwIDAQABAoIBAAV+fcW5oIX2gQ3Tosu68ZvAI2r8nByv9ovb9CeMWCqmZJXaKr1S8z8qDjQzrmN5qLgAedaoqn2KlSpwXxAAYizxCY75EYK6HgrAszCF++OqNfANq7U5/uZaD37bIlwKrRRGo1kh7oMPW8yPil/YDBI+hv91Fi0uvrsT5nz26RyzbgRjSWtFhsrFvuYH/Y190048x0GX9d+m3RVhfautchha2eS7k04oEDgnAjmXHlUJF6v/9jIQwzzerrAYNTxwXhwhht/m4+zfR+EA5y737RIPXytUqIyTBZAsaNGe+jsrJiEWQLgjTdYN5tglhZr9y5QdVAKOtS0Vp1qvPtR9SsECgYEA/VE3M60/dMkw3BhhHDZ0mBTDPF8U87sQWk5/Wkc3a541gLTCIAG9SjaexK6VEMI+lFMETwqXd3H1eu50KQzmuEadJSIoOAqdwz6BpCoUXURJhgyZ9tSqS8HNP77uGEQGvTqNBsJUq8mFzF3XqExtGhsWcZu23TaIupu5Q/ERrbsCgYEA8pZH5m3weztU0ASzYOxgJPhkgwEQ71vKoD92BWJsrKT9l9LcjgMIryIgPs3pHwHhNqE5Jz1DxPmSbBHAvoYCpYd/q3nhaVD8xr9hmToqU3n436V7/pbnKqVfsl9w9k0NrOXfH62lU4XaooAXqyJzp01oDHnZEEuAqJAGCI0qGokCgYEAmmXLGKNLHxqVvkDZx8FlNWDKUty9P4VQBUr9EoB/xgdrIuSJwcptnnNGOCkU0F8kl3E2GhshkPsTnXcDtBWZ9s2LcJuuGeJWoC9UA0RmnyZFDgBWQ30BwyT/vg/YvU+RL/slA+6kfwRYmeTE0l6qzROfjWVUguZ/VgYeWMvxc4sCgYAHeD0qiUQYZ1FLPnnPiDfFq6hcqFG/LRJmMM4PiGJURphQbnvAG7POwLxBctG01Lf8mKZL4QUPMagij8FhnHrEYhG6U/xu48GE1SvKLhJTk7PoHw5cXoUruP/jXDD8RI8hPcrLDOvEUKDU1CDmch8hja6gg7f9WnXRuu2UodQkEQKBgDyr19w7uUR38irP52IegyQrnalII1yLe6IP7kMJQ9fVd8I8jHwWXBoqUAjhAd/N1Eg++tuALGGHE5VinL6e9Ukh4nWoC5M2e6Z2uG44/XiwBKZN4tQ+2Xuh83nClilfrIR9TJCI4dm7XNbTwxaxbwVK6xbcQyxO8LsS12nE8ddF-----END RSA PRIVATE KEY-----";

MString _ssl_init_0, _ssl_init_1, _ssl_init_2;

int SslProgSecureInit(){
	MkDir("secure");

	// create
	if(!IsFile("secure/_init_0.key")){
		SaveFile("secure/do_not_delete_this_files.txt", "Else all data will be lose!");
		SaveFile("secure/_init_0.key", RandGetUniqLine());
	}

	_ssl_init_0 = LoadFile("secure/_init_0.key");

	// create RSA keys
	if(!IsFile("secure/_init_1.key") || !IsFile("secure/_init_2.key")){
		RsaSaveCreateKeys("secure/_init_1.key", "secure/_init_2.key");
		SaveFile("secure/_init_1.key", AesEncode(LoadFile("secure/_init_1.key"), _ssl_init_0, _ssl_init_p));
		SaveFile("secure/_init_2.key", AesEncode(LoadFile("secure/_init_2.key"), _ssl_init_0, _ssl_init_s));
	}

	_ssl_init_1 = LoadFile("secure/_init_1.key");
	_ssl_init_2 = LoadFile("secure/_init_2.key");
	
	return _ssl_init_0 && _ssl_init_1 && _ssl_init_2;
}


TString AesEncodeMyDataBlock(VString data, VString ckey, VString ivek){
	TString ret;
	
	TString key = RandGetUniqLine();
	TString key2 = RandGetUniqLine();

	key += VString(key2).str(0, 100);
	return ret.Add(VString(key).str(0, 256), RsaPublicEncode(ckey, VString(key2).str(0, 100)), AesEncode(data, key, ivek));
}

TString AESDecodeMyDataBlock(VString data, VString ckey, VString ivek){
	TString key = data.str(0, 256);
	VString key2 = data.str(256, 256);
	VString line = data.str(512);

	key.Add(key, RsaPrivateDecode(ckey, key2));
	return AesDecode(line, key, ivek);
}


TString TEncodeData(VString data){
	TString ret;
	
	TString key = RandGetUniqLine();
	TString key2 = RandGetUniqLine();

	key += VString(key2).str(0, 100);
	return ret.Add(VString(key).str(0, 256), RsaPublicEncode(AesDecode(_ssl_init_1, _ssl_init_0, _ssl_init_p), VString(key2).str(0, 100)), AesEncode(data, key, _ssl_init_1));
}

TString TDecodeData(VString data){
	TString key = data.str(0, 256);
	VString key2 = data.str(256, 256);
	VString line = data.str(512);

	key.Add(key, RsaPrivateDecode(AesDecode(_ssl_init_2, _ssl_init_0, _ssl_init_s), key2));
	return AesDecode(line, key, _ssl_init_1);
}

/*
int DecodeDeflateDo(z_stream &zs, VString data, VString ret){
	memset(&zs, 0, sizeof(zs));
	inflateInit(&zs);
	zs.avail_in=data; zs.next_in=data; zs.avail_out=ret; zs.next_out=ret;
	int r=inflate(&zs, Z_NO_FLUSH);
	r = inflate(&zs, Z_FINISH);
	return r;
}

MString DecodeDeflate(VString data){
	if(!data) return MString();

	unsigned char dbuf[S512K], *rbuf=dbuf; MString b; z_stream zs;

	int r=DecodeDeflateDo(zs, data, VString(dbuf, sizeof(dbuf)));

	if(r==-5){
		b.Reserv(S16M); rbuf=b;
		r=DecodeDeflateDo(zs, data, b);
	}

	if(r==-5){
		print("NO MEMORY!!!!!!!!!!!!!!!!\r\n");
		return MString();
	}

	if(r<0){
		print("INFLATE FAIL\r\n");
//		AddEventDo(ZFNENUM_BADDATA);
		return MString();
	}

	return MString(rbuf, zs.total_out);
}*/

int OpenSSLCreateRSAPrivateKey(TString &r_key, int bits){
	RSA * rsa;
	rsa = RSA_generate_key(
		bits,   /* number of bits for the key - 2048 is a sensible value */
		RSA_F4, /* exponent - RSA_F4 is defined as 0x10001L */
		NULL,   /* callback - can be NULL if we aren't displaying progress */
		NULL    /* callback argument - not needed in this case */
	);

	//EVP_PKEY_assign_RSA(pkey, rsa);

	// save key
	int sz;
	BIO *bio = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(bio, rsa, 0, 0, 0, 0, 0);

	sz = BIO_ctrl_pending(bio);
	r_key.Reserve(sz);
	BIO_read(bio, r_key, sz);
	
	BIO_free(bio);
	RSA_free(rsa);

	return sz;
}

int OpenSSLCreateCertificateRequest(VString r_key, TString &r_req, VString ca, VString co, VString cn){
	// http://stackoverflow.com/questions/256405/programmatically-create-x509-certificate-using-openssl
	
	EVP_PKEY * pkey;
	int ret = 0;

	pkey = EVP_PKEY_new();

	// read private key
	BIO *b = BIO_new_mem_buf(r_key, r_key), *bio;

	int r = 0, sz;
	if(!PEM_read_bio_PrivateKey(b, &pkey, 0, 0)){
		BIO_free(b);
		EVP_PKEY_free(pkey);
		return 0;
	}

	BIO_free(b);

	// create certificate
	X509_REQ * x509;
	x509 = X509_REQ_new();

	X509_REQ_set_version(x509, 2);
	//ASN1_INTEGER_set(X509_REQ_get_serialNumber(x509), 1);
	//X509_gmtime_adj(X509_REQ_get_notBefore(x509), 0);
	//X509_gmtime_adj(X509_REQ_get_notAfter(x509), 31536000L);

	X509_REQ_set_pubkey(x509, pkey);

	X509_NAME * name;
	name = X509_REQ_get_subject_name(x509);

	X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)ca, ca, -1, 0);
	X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)co, co, -1, 0);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)cn, cn, -1, 0);

	// X509_set_issuer_name(x509, name);

	if(!X509_REQ_sign(x509, pkey, EVP_sha256()))
		goto free_all;

	// save cert
	bio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509_REQ(bio, x509);
	sz = BIO_ctrl_pending(bio);
	r_req.Reserve(sz);
	BIO_read(bio, r_req, sz);
	BIO_free(bio);

	ret = 1;

free_all:
	X509_REQ_free(x509);
	EVP_PKEY_free(pkey);
	//OPENSSL_free(name);
	//RSA_free(rsa);

	return ret;
}

int OpenSSLCreateCACertificate(VString r_key, TString &r_req, VString ca, VString co, VString cn){
	// http://stackoverflow.com/questions/256405/programmatically-create-x509-certificate-using-openssl
	// http://www.codepool.biz/security/how-to-use-openssl-to-sign-certificate.html
	
	EVP_PKEY * pkey;
	int ret = 0;

	pkey = EVP_PKEY_new();

	// read private key
	BIO *b = BIO_new_mem_buf(r_key, r_key), *bio;

	int r = 0, sz;
	if(!PEM_read_bio_PrivateKey(b, &pkey, 0, 0)){
		BIO_free(b);
		EVP_PKEY_free(pkey);
		return 0;
	}

	BIO_free(b);

	// create certificate
	X509 * x509;
	x509 = X509_new();

	X509_set_version(x509,2);
	ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
	X509_gmtime_adj(X509_get_notBefore(x509), 0);
	X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

	X509_set_pubkey(x509, pkey);

	X509_NAME * name;
	name = X509_get_subject_name(x509);

	X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)ca, ca, -1, 0);
	X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)co, co, -1, 0);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)cn, cn, -1, 0);

	X509_set_issuer_name(x509, name);

	if(!X509_sign(x509, pkey, EVP_sha256()))
		goto free_all;

	// save cert
	bio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(bio, x509);
	sz = BIO_ctrl_pending(bio);
	r_req.Reserve(sz);
	BIO_read(bio, r_req, sz);
	BIO_free(bio);

	ret = 1;

free_all:
	X509_free(x509);
	EVP_PKEY_free(pkey);
	//OPENSSL_free(name);
	//RSA_free(rsa);

	return ret;
}

bool OpenSSLSignCertificateLoadCA(X509 ** px509, VString ca_cert){
	// read ca
	BIO *b = BIO_new_mem_buf(ca_cert, ca_cert);
	bool ret = (PEM_read_bio_X509(b, px509, 0, 0)!=0);
	BIO_free(b);
    return ret;
}

// load ca private key
bool OpenSSLSignCertificateLoadCAPrivateKey(EVP_PKEY **ppkey, VString ca_key){
    bool ret;
    BIO *in = NULL;
    RSA *r = NULL;
    EVP_PKEY *pkey = NULL;

	in = BIO_new_mem_buf(ca_key, ca_key);
    ret = (PEM_read_bio_RSAPrivateKey(in, &r, NULL, NULL) != NULL);
    if(!ret)
        goto free_all;
 
    pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, r);
    *ppkey = pkey;
    r = NULL;
 
free_all:
    BIO_free(in);
    return ret;
}
 
// load X509 Req
bool OpenSSLSignCertificateLoadX509Req(X509_REQ **ppReq, VString req){
    bool ret;
    BIO *in = NULL;
 
    in = BIO_new_mem_buf(req, req);
    ret = (PEM_read_bio_X509_REQ(in, ppReq, NULL, NULL) != NULL);
 
    BIO_free(in);
    return ret;
}

int OpenSSLSignCertificateDoX509Sign(X509 *cert, EVP_PKEY *pkey, const EVP_MD *md){
    EVP_MD_CTX *mctx;
	int rv;

#if OPENSSL_VERSION_NUMBER >= 0x1010000fL
    mctx = EVP_MD_CTX_new();
#else
	EVP_MD_CTX _mctx;
	mctx = &_mctx;
	EVP_MD_CTX_init(mctx);
#endif
    EVP_PKEY_CTX *pkctx = NULL;

    rv = EVP_DigestSignInit(mctx, &pkctx, md, NULL, pkey);
 
    if (rv > 0)
        rv = X509_sign_ctx(cert, mctx);

#if OPENSSL_VERSION_NUMBER >= 0x1010000fL
	EVP_MD_CTX_free(mctx);
#else
	EVP_MD_CTX_cleanup(mctx);
#endif
    
	return rv > 0 ? 1 : 0;
}

int OpenSSLSignCertificate(TString &r_cert, VString ca_cert, VString ca_key, VString r_req, int serial){
	int ret = 0, sz;
	X509 * ca = NULL;
    X509_REQ * req = NULL;
    EVP_PKEY *pkey = NULL, *pktmp = NULL;
 
    X509_NAME *subject = NULL, *tmpname = NULL;
    X509 * cert = NULL;
    BIO *out = NULL;

	//int serial = 1;
    long days = 3650 * 24 * 3600 / 2; // 10 years

	if(!OpenSSLSignCertificateLoadCA(&ca, ca_cert))
        goto free_all;
 
    if(!OpenSSLSignCertificateLoadCAPrivateKey(&pkey, ca_key))
        goto free_all;
 
    if(!OpenSSLSignCertificateLoadX509Req(&req, r_req))
        goto free_all;

	    cert = X509_new();
    // set version to X509 v3 certificate
    if (!X509_set_version(cert,2))
        goto free_all;
 
    // set serial
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
 
    // set issuer name frome ca
    if (!X509_set_issuer_name(cert, X509_get_subject_name(ca)))
        goto free_all;
 
    // set time
    X509_gmtime_adj(X509_get_notBefore(cert), -3600);
    X509_gmtime_adj(X509_get_notAfter(cert), days);
 
    // set subject from req
    tmpname = X509_REQ_get_subject_name(req);
    subject = X509_NAME_dup(tmpname);
    if (!X509_set_subject_name(cert, subject))
        goto free_all;
 
    // set pubkey from req
    pktmp = X509_REQ_get_pubkey(req);
    ret = X509_set_pubkey(cert, pktmp);
    EVP_PKEY_free(pktmp);
    if (!ret) goto free_all;
 
    // sign cert
    if (!OpenSSLSignCertificateDoX509Sign(cert, pkey, EVP_sha256()))
        goto free_all;
 
    //out = BIO_new_file(szUserCert,"w");
    //ret = PEM_write_bio_X509(out, cert);

	// save cert
	out=BIO_new(BIO_s_mem());
	PEM_write_bio_X509(out, cert);
	sz=BIO_ctrl_pending(out);
	r_cert.Reserve(sz);
	ret=BIO_read(out, r_cert, sz);
	//BIO_free(bio);
 
free_all:
 
    X509_free(cert);
    BIO_free_all(out);
 
    X509_REQ_free(req);
    X509_free(ca);
    EVP_PKEY_free(pkey);

	return (ret > 0);
}



// ECDSA
void EcdsaCreateSecretKey(TString &sec){
	// generate secret key

	sec.Clean();

	EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
	if(!key)
		return ;

	if(!EC_KEY_generate_key(key)){
		EC_KEY_free(key);
		return ;
	}

	//int sz = i2d_ECPrivateKey(key, 0);

	//if(!sec.Reserve(sz)){
	//	EC_KEY_free(key);
	//	return ;
	//}

	//int ret = i2d_ECPrivateKey(key, &sec.data);
	//if(ret <= 0){
	//	EC_KEY_free(key);
	//	sec.Clean();		
	//	return ;
	//}

	const BIGNUM *skey = EC_KEY_get0_private_key(key);
	if(!skey){
		EC_KEY_free(key);
		return ;
	}

	char* hkey = BN_bn2hex(skey);
    if(hkey && sec.set(hkey) && sec)
		{}
	else
		sec.Clean();

	EC_KEY_free(key);

	return ;
}

TString EcdsaCreatePublicKey(VString sec){
	// generate public key
	TString pub;

	if(!sec)
		return TString();

	EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
	if(!key)
		return TString();

	BIGNUM *bnkey = BN_new();

	if(!BN_hex2bn(&bnkey, sec)){
		EC_KEY_free(key);
		BN_free(bnkey);
		return TString();
	}

	if(!EC_KEY_set_private_key(key, bnkey)){
		EC_KEY_free(key);
		BN_free(bnkey);
		return TString();
	}

	// Generate public key
	const EC_GROUP *group = EC_KEY_get0_group(key);
	EC_POINT *pkey = EC_POINT_new(group);

	if (!EC_POINT_mul(group, pkey, bnkey, NULL, NULL, NULL))
       printf("Error at EC_POINT_mul.\n");

     EC_KEY_set_public_key(key, pkey);

	 const EC_POINT *bpkey = EC_KEY_get0_public_key(key);
	 char *hkey = EC_POINT_point2hex(group, bpkey, POINT_CONVERSION_UNCOMPRESSED, 0);

	 //char *hkey = EC_POINT_point2hex(group, pkey, 4, 0);

	if(hkey && pub.set(hkey) && pub)
		{}
	else
		pub.Clean();


	EC_KEY_free(key);

	return pub;
}

void EcdsaCreateKeys(TString &pub, TString &sec){
	EcdsaCreateSecretKey(sec);
	pub = EcdsaCreatePublicKey(sec);
	return ;
}

TString EcdsaGetAddr(VString pub){
	MHash mh(MHASHT_SHA256);
	TString sha, rmd, addr, res;

	sha = mh.FastH(MHASHT_SHA256, htob(pub));
	rmd = mh.FastH(MHASHT_RMD160, htob(sha));
	addr.Add("00", rmd);
	sha = mh.FastH(MHASHT_SHA256, htob(addr));
	sha = mh.FastH(MHASHT_SHA256, htob(sha));

	addr.Add(addr, sha.str(0, 8));
	res = Base64::btos58(htob(addr));

	return res;
}


// Test Bitcoin Address
void TestBitconAddress(){
	print("Test Bitcoin Address.\r\n");

	MHash mh(MHASHT_SHA256);
	TString pub, sec, s256, r160, net, seven, addr, addrb;
	VString res;

	EcdsaCreateKeys(pub, sec);
	print("ECDSA Priv: ", sec, "\r\n", "Pub: ", pub, "\r\n");
	
	s256 = mh.FastH(MHASHT_SHA256, htob(pub));
	print("2) Sha256: ", s256, "\r\n");

	r160 = mh.FastH(MHASHT_RMD160, htob(s256));
	print("3) Rmd160: ", r160, "\r\n");

	net.Add("00", r160);
	print("4) Net: ", net, "\r\n");

	s256 = mh.FastH(MHASHT_SHA256, htob(net));
	print("5) Sha256: ", s256, "\r\n");

	s256 = mh.FastH(MHASHT_SHA256, htob(s256));
	print("6) Sha256: ", s256, "\r\n");

	seven = s256.str(0, 8);
	print("7) First four bytes of 6: ", seven, "\r\n");
	
	addr.Add(net, seven);
	print("8) Addr: ", addr, "\r\n");

	addrb = Base64::btos58(htob(addr));
	print("9) Base Addr: ", addrb, "\r\n");

	return ;
}


// https://bitcoin.stackexchange.com/questions/52434/building-signed-bitcoin-transaction-in-java/52731
class TestBitcoinTx{
	MString data;

	int ver;
	uint64 in_count, out_count;

public:
	int ReadHex(VString line){
		return Read(htob(line));
	}

	int Read(VString line){
		if(line.sz < 8)
			return 0;

		unsigned char *ln = line, *to = line.endu();

		// Version
		memcpy(&ver, ln, 4);
		ln += 4;

		// Input count
		int len = ReadVarInt(ln, to, in_count);
		if(len < 0)
			return 0;

		ln += len;

		// Read inputs
		for(uint64 i = 0; i < in_count; i ++){
			// Read prevous hash & index
			if(ln + 32 + 4 > to)
				return 0;

			VString phash(ln, 32);
			ln += 32;

			unsigned int pindex;
			memcpy(&pindex, ln, 4);
			ln += 4;

			// Script
			if(ln + 1 > to)
				return 0;

			unsigned int slen = 0;
			memcpy(&slen, ln, 1);
			ln += 1;

			if(ln + slen > to)
				return 0;

			VString script(ln, slen);

			ln += slen;

			// Sequence
			if(ln + 4 > to)
				return 0;

			unsigned int seq = 0;
			memcpy(&seq, ln, 4);
			ln += 4;
		}

		// Output count
		len = ReadVarInt(ln, to, out_count);
		if(len < 0)
			return 0;

		ln += len;

		for(uint64 i = 0; i < out_count; i ++){
			// Pay 
			if(ln + 4 > to)
				return 0;

			uint64 pay = 0;
			memcpy(&pay, ln, 8);
			ln += 8;

			// Script len
			uint64 slen = 0;

			len = ReadVarInt(ln, to, slen);
			if(len < 0)
				return 0;

			ln += len;

			if(ln + slen > to)
				return 0;

			VString script(ln, slen);

			ln += slen;
		}

		// Lock time
		if(ln + 4 > to)
			return 0;

		unsigned int ltime = 0;
		memcpy(&ltime, ln, 4);
		ln += 4;

		if(ln != to)
			return 0;

		return 1;
	}

	int ReadVarInt(unsigned char *ln, unsigned char *to, uint64 &res){
		if(*ln < 0xFD && ln + 1 <= to){
			res = *ln;
			return 1;
		} else if(*ln == 0xFD && ln + 3 <= to){
			res = *(unsigned short*)(ln + 1);
			return 3;
		} else if(*ln == 0xFE && ln + 5 <= to){
			res = *(unsigned int*)(ln + 1);
			return 5;
		} else if(*ln == 0xFF && ln + 9 <= to){
			res = *(uint64*)(ln + 1);
			return 9;
		}

		return -1;
	}


};

void TestBitcoinSig(){
	TestBitcoinTx tx;
	MHash mh(MHASHT_SHA256);
	VString s;
	int res;
		
	s = "0100000001e468833270cf713f3bbccc62b7b5b0fc0b0a4570608718530816795a6589f322000000008a473044022051646b77924f6bb7c411c5aa890110ab55db8812b8998fe24c8bdce39795ebd602200bc4de18fd5524ad8b946ee57604424e2b943ef2a14fc7199a7853dda0743cbe014104b97c679207532e0f4ee2515aedaba5f87700bbe0138f7457baa58e89a53153823ab29632e6c3c804ecaab5913656512339792479a1b898b7e5dc31f075ff8660ffffffff0176df1710000000001976a91448ddfd3891f3f422d5c3c9c25e35b382667fc6e688ac00000000";
	s = "0100000001e468833270cf713f3bbccc62b7b5b0fc0b0a4570608718530816795a6589f322000000001976a91448ddfd3891f3f422d5c3c9c25e35b382667fc6e688acffffffff0176df1710000000001976a91448ddfd3891f3f422d5c3c9c25e35b382667fc6e688ac00000000";
	//s = "01000000018dd4f5fbd5e980fc02f35c6ce145935b11e284605bf599a13c6d415db55d07a1000000001976a91446af3fb481837fadbb421727f9959c2d32a3682988acffffffff0200719a81860000001976a914df1bd49a6c9e34dfa8631f2c54cf39986027501b88ac009f0a5362000000434104cd5e9726e6afeae357b1806be25a4c3d3811775835d235417ea746b7db9eeab33cf01674b944c64561ce3388fa1abd0fa88b06c44ce81e2234aa70fe578d455dac0000000";
	//res = tx.ReadHex(s);

	TString h, sha, ret;
	//h.Add(htob(s), htob("01000000"));
	//mh.Add(h);
	//mh.Finish();
	//sha = mh.Get();




	sha = mh.FastH(MHASHT_SHA256, htob(h.Add(s, "01000000")));
	sha = mh.FastH(MHASHT_SHA256, htob(sha));
	//sha = mh.FastH(MHASHT_SHA256, h);

	//VString q = "083867478cb0d1d8bb864175bbc49728cffcc114bc2e762c6df64f2c965a9a66";
	//VString q = "405a1f95ed042176c6360a290efd59de8926218d6290cd555b2c5d98bb2e2ace";

	

	// sha
	// 7ceb9e6
	// cb8cd64

	// ret
	// e6c24a33
	// c55ff445



	return ;
}