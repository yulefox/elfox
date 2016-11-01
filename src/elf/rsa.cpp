#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <string>
#include <elf/base64.h>

namespace elf {

int RSAVerify(std::string ctx, std::string sign, std::string pubkey)
{
    BIO* mem_bio = NULL;
    RSA *rsa = NULL;

    if ((mem_bio = BIO_new_mem_buf(pubkey.c_str(), -1)) == NULL) {
        BIO_free(mem_bio);
        return -1;
    }

    rsa = PEM_read_bio_RSA_PUBKEY(mem_bio, NULL, NULL, NULL);
    if (rsa == NULL) {
        BIO_free(mem_bio);
        return -1;
    }

    unsigned char buf[1024] = {0};
    int size = b64_decode((const char*)sign.c_str(), sign.length(), buf);

	unsigned char sum[1024] = {0};
	SHA256((unsigned char*)ctx.c_str(), ctx.length(), sum);
    int ret = RSA_verify(NID_sha256, sum, SHA256_DIGEST_LENGTH, (unsigned char*)buf, size, rsa);
    BIO_free(mem_bio);
    return ret;
}


} // namespace elf
