#include "security.h"
#include "base.h"
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/hmac.h>
#include "b64/urlsafe_b64.h"

int GetHttpRequestSign(const char * pKey, int nKeyLen, char *method, char *pUrlWithPathAndQuery, char *pContentType,
                              char *pData, int nDataLen, char *pOutput, int *pOutputLen) {
        int isHttps = 0;
        if (memcmp(pUrlWithPathAndQuery, "http", 4) == 0) {
                if (pUrlWithPathAndQuery[4] == 's')
                        isHttps = 1;
        }
        char *hostStart = pUrlWithPathAndQuery+7+isHttps;
        char *hostEnd = strchr(hostStart, '/');
        if (hostEnd == NULL) {
                LinkLogError("no path in url:", pUrlWithPathAndQuery);
                return LINK_ARG_ERROR;
        }
        
        int nBufLen = strlen(pUrlWithPathAndQuery);
        assert(nBufLen < 192); //TODO
        nBufLen = nBufLen + nDataLen + 256;
        char *buf = malloc(nBufLen);
        if (buf == NULL) {
                return LINK_NO_MEMORY;
        }
        
        int nOffset = snprintf(buf, nBufLen, "%s ", method);

        nOffset+= snprintf(buf+nOffset, nBufLen - nOffset, "%s", hostEnd);
        
        int nHostLen = hostEnd - hostStart;
        nOffset+= snprintf(buf+nOffset, nBufLen - nOffset, "\nHost: ");
        memcpy(buf + nOffset, hostStart, nHostLen);
        nOffset += nHostLen;

        if (pContentType) {
                nOffset+= snprintf(buf+nOffset, nBufLen - nOffset, "\nContent-Type: %s", pContentType);
        }
        
        buf[nOffset++] = '\n';
        buf[nOffset++] = '\n';
        if (nDataLen > 0) {
                memcpy(buf + nOffset, pData, nDataLen);
                nOffset += nDataLen;
        }
        
        char sha1[20];
        int nSha1 = 20;
        int ret = HmacSha1(pKey, nKeyLen, buf, nOffset, sha1, &nSha1);
        free(buf);
        
        int outlen = urlsafe_b64_encode(sha1, 20, pOutput, *pOutputLen);
        *pOutputLen = outlen;
        return ret;
}

/*hmac-sha1 output is 160bit(20 byte)*/
int HmacSha1(const char * pKey, int nKeyLen, const char * pInput, int nInputLen,
        char *pOutput, int *pOutputLen) { //EVP_MAX_MD_SIZE
#if 0
        unsigned int outlen=20;
        unsigned char * res = HMAC(EVP_sha1(), (const unsigned char *)pKey, nKeyLen,
                                   (const unsigned char *)pInput, nInputLen,
                                   (unsigned char *)pOutput, &outlen);
        if (res == NULL) {
                return LINK_WOLFSSL_ERR;
        }
        *pOutputLen = 20;
        return LINK_SUCCESS;
        
#endif

#if 1
        Hmac hmac;
        memset(&hmac, 0, sizeof(hmac));
        int ret = 0;
        
        ret = wc_HmacSetKey(&hmac, SHA, (byte*)pKey, nKeyLen);
        if (ret != 0) {
                return LINK_WOLFSSL_ERR;
        }

        if( (ret = wc_HmacUpdate(&hmac, (byte*)pInput, nInputLen)) != 0) {
                return LINK_WOLFSSL_ERR;
        }
        
        if ((ret = wc_HmacFinal(&hmac, (byte*)pOutput)) != 0) {
                return LINK_WOLFSSL_ERR;
        }
        *pOutputLen = 20;
        return LINK_SUCCESS;
#endif

#if 0
        int ret = 0;
        Sha sha;
        
        ret = wc_InitSha(&sha);
        if (ret != 0) {
                return LINK_WOLFSSL_ERR;
        }
        INVALID_DEVID;
        wc_HmacInit();
        ret = wc_ShaUpdate(&sha, (unsigned char*)pInput, nInputLen);
        if (ret != 0) {
                wc_ShaFree(&sha);
                return LINK_WOLFSSL_ERR;
        }
        
        ret = wc_ShaFinal(&sha, (byte *)pOutput);
        if (ret != 0) {
                wc_ShaFree(&sha);
                return LINK_WOLFSSL_ERR;
        }
        wc_ShaFree(&sha);
        *pOutputLen = 20;
        
        return LINK_SUCCESS;
#endif

#if 0
        int ret = 0;
        EVP_MD_CTX md_ctx;
        
        EVP_MD_CTX_init(&md_ctx);

        ret = EVP_DigestInit(&md_ctx, EVP_sha1());
        if (ret == 0) {
                 EVP_MD_CTX_cleanup(&md_ctx);
                return LINK_WOLFSSL_ERR;
        }
        
        ret = EVP_DigestUpdate(&md_ctx, (unsigned char*)pInput, nInputLen);
        if (ret == 0) {
                EVP_MD_CTX_cleanup(&md_ctx);
                return LINK_WOLFSSL_ERR;
        }
        
        unsigned int nOutLen = 0;
        ret = EVP_DigestFinal(&md_ctx, (unsigned char*)pOutput, &nOutLen);
        if (ret == 0) {
                 EVP_MD_CTX_cleanup(&md_ctx);
                return LINK_WOLFSSL_ERR;
        }
         EVP_MD_CTX_cleanup(&md_ctx);
        *pOutputLen = (int)nOutLen;
        return LINK_SUCCESS;
#endif
        
#if 0
        const EVP_MD * engine = engine = EVP_sha1();
        int ret = 0;
  
        HMAC_CTX ctx ;
        memset(&ctx, 0, sizeof(ctx));
        ret = HMAC_CTX_init(&ctx);
        if (ret == 0) {
                return LINK_WOLFSSL_ERR;
        }
        ret = HMAC_Init_ex(&ctx, pKey, nKeyLen, engine, NULL);
        if (ret == 0) {
                return LINK_WOLFSSL_ERR;
        }
        ret = HMAC_Update(&ctx, (unsigned char*)pInput, nInputLen);
        if (ret == 0) {
                return LINK_WOLFSSL_ERR;
        }
        
        unsigned int nOutLen = 0;
        ret = HMAC_Final(&ctx, (unsigned char*)pOutput, &nOutLen);
        if (ret == 0) {
                return LINK_WOLFSSL_ERR;
        }
        HMAC_cleanup(&ctx);
        *pOutputLen = (int)nOutLen;
        return LINK_SUCCESS;
#endif
}


int GetMqttPasswordSign(IN const char *_pInput, IN int _nInLen,
                OUT char *_pOutput, OUT int *_pOutLen, IN const char *_pDsk)
{
        int ret = 0;
        int sha1Len = 20;
        char sha1[256] = {0};

        if (!_pInput || !_pDsk) {
                return LINK_ERROR;
        }
        ret = HmacSha1(_pDsk, strlen(_pDsk), _pInput, _nInLen, sha1, &sha1Len);
        if (ret != 0) {
                return LINK_ERROR;
        }
        int outlen = urlsafe_b64_encode(sha1, 20, _pOutput, _pOutLen);
        *_pOutLen = outlen;
        return LINK_SUCCESS;
}


int GetMqttUsernameSign(OUT char *_pUsername, OUT int *_pLen, IN const char *_pDak)
{
        char query[256] = {0};
        long timestamp = 0.0;
        timestamp = (long)time(NULL);
        if (!_pDak) {
                return LINK_ERROR;
        }
        sprintf(query, "dak=%s&timestamp=%ld&version=v1", _pDak, timestamp);
        *_pLen = strlen(query);
        memcpy(_pUsername, query, *_pLen);
        return LINK_SUCCESS;
}

