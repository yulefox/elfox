#include <openssl/md5.h>
#include <elf/md5.h>
#include <string>

namespace elf {
std::string md5(const unsigned char *d, unsigned long n) {
    unsigned char md[MD5_DIGEST_LENGTH+1];
    std::string res;
    MD5(d, n, md);
    for (int i = 0;i < MD5_DIGEST_LENGTH; i++) {
        char hex[3];
        sprintf(hex, "%02x", md[i]);
        res.append(hex);
    }
    return res;
}
} // namespace elf
