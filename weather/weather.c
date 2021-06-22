#include "weather.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <time.h>

#include "cJSON/cJSON.h"

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>

static char config_location[128];
static char config_key[128];

static weather_data_t weather_data = {.code = 99, .temp = 0};

char *base64Encode(const char *buffer, int length, bool newLine);
int URLEncode(const char *str, const int strSize, char *result, const int resultSize);
void seniverse_v3(char *buf, int size, char *key, char *location);
void seniverse_v4(char *buf, int size, char *pubic_key, char *private_key);

void weather_init(config_t *config)
{
    strcpy(config_location, config->location);
    strcpy(config_key, config->key);
}

void seniverse_v3(char *buf, int size, char *private_key, char *location)
{
    bzero(buf, size);
    snprintf(buf,
             size,
             "GET /v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c HTTP/1.1\r\n"
             //  "GET /v3/weather/now.json?key=%s&location=%s&language=en&unit=c HTTP/1.1\r\n"
             "Connection: Close\r\n"
             "HOST: api.seniverse.com\r\n\r\n",
             private_key,
             location);
}

#if USE_API_V4
void seniverse_v4(char *buf, int size, char *pubic_key, char *private_key)
{
    char url[512] = {'\0'};
    unsigned char sig[128] = {'\0'};
    uint sig_len = 0;
    time_t ts = 0;

    bzero(buf, size);

    time(&ts);
    sprintf(url, "fields=weather_daily&locations=beijing&public_key=%s&ts=%ld&ttl=300", pubic_key, ts);

    printf("origin:%s\r\n", url);
    printf("key:%s\r\n", private_key);

    HMAC(EVP_sha1(), private_key, strlen(private_key), url, strlen(url), sig, &sig_len);

    printf("HMAC_sha1:");
    for (int i = 0; i < sig_len; i++)
    {
        printf("%02x", sig[i]);
    }
    printf("\r\n");

    char *base64_ret;
    base64_ret = base64Encode(sig, sig_len, false);
    printf("base64:%s\r\n", base64_ret);

    unsigned char urlencode[128] = {'\0'};
    URLEncode(base64_ret, strlen(base64_ret), urlencode, sizeof(urlencode));
    printf("URLEncode:%s\r\n", urlencode);

    snprintf(buf,
             size,
             "GET /v4?%s&sig=%s HTTP/1.1\r\n"
             "Connection: Close\r\n"
             "HOST: api.seniverse.com\r\n\r\n",
             url, urlencode);

    printf("request:%s\r\n", buf);

    printf("www:https://api.seniverse.com/v4?%s&sig=%s\r\n", url, urlencode);
}

char *base64Encode(const char *buffer, int length, bool newLine)
{
    BIO *bmem = NULL;
    BIO *b64 = NULL;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    if (!newLine)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, buffer, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    BIO_set_close(b64, BIO_NOCLOSE);

    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;
    BIO_free_all(b64);

    return buff;
}

int URLEncode(const char *str, const int strSize, char *result, const int resultSize)
{
    int i;
    int j = 0; //for result index
    char ch;

    if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0))
    {
        return 0;
    }

    for (i = 0; (i < strSize) && (j < resultSize); ++i)
    {
        ch = str[i];
        if (((ch >= 'A') && (ch <= 'Z')) ||
            ((ch >= 'a') && (ch <= 'z')) ||
            ((ch >= '0') && (ch <= '9')) ||
            ch == '.' || ch == '-' || ch == '_' || ch == '*')
        {
            result[j++] = ch;
        }
        else if (ch == ' ')
        {
            result[j++] = '+';
        }
        else
        {
            if (j + 3 < resultSize)
            {
                sprintf(result + j, "%%%02X", (unsigned char)ch);
                j += 3;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}
#endif

static char buffer[1024] = {'\0'};

weather_data_t *weather_data_upate()
{
    int s = 0;
    struct sockaddr_in servaddr;
    struct hostent *host = NULL;

    printf("geting host\r\n");
    host = gethostbyname("api.seniverse.com");
    if (host != NULL)
    {
        printf("get host success\r\n");
        printf("h_name:%s\r\n", host->h_name);
        printf("h_length:%d\r\n", host->h_length);

        for (uint8_t i = 0; host->h_addr_list[i]; i++)
        {
            printf("%s\r\n", inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
        }
    }
    else
    {
        printf("get host fail\r\n");
        return NULL;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1)
    {
        printf("get socket fail\r\n");
        return NULL;
    }
    else
    {
        printf("get socket success\r\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);

    memcpy(&servaddr.sin_addr, host->h_addr_list[0], sizeof(struct in_addr));

    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("socket connect fail\r\n");
        return NULL;
    }
    else
    {
        printf("socket connect success\r\n");
    }

    seniverse_v3(buffer, 1024, config_key, config_location);

    // seniverse_v4(buffer, 4096, pubic_key, private_key);

    if (send(s, buffer, strlen(buffer), 0) == -1)
    {
        printf("socket send fail\r\n");
        return NULL;
    }
    else
    {
        printf("socket send success\r\n");
    }

    uint len = 0;
    bzero(buffer, sizeof(buffer));

    while (1)
    {
        printf("waiting for data\r\n");

        if ((len = recv(s, buffer, sizeof(buffer), 0)) <= 0)
        {
            printf("no data\r\n");
            break;
        }
        printf("%d\r\n", len);
    }

    printf("\r\nbuffer:\r\n");
    printf("%s\r\n\r\n", buffer);

    close(s);
    printf("connection closed\r\n");

    if (strstr(buffer, "HTTP/1.1 200 OK"))
    {
        cJSON *root = NULL;

        root = cJSON_Parse(strstr(buffer, "\r\n\r\n"));

        if (root != NULL)
        {
            cJSON *results = cJSON_GetObjectItem(root, "results");

            printf("%s\r\n", cJSON_Print(results));

            cJSON *location = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "location");
            cJSON *now = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "now");

            cJSON *city = cJSON_GetObjectItem(location, "name");

            cJSON *text = cJSON_GetObjectItem(now, "text");
            cJSON *code = cJSON_GetObjectItem(now, "code");
            cJSON *temp = cJSON_GetObjectItem(now, "temperature");

            printf("\r\ncity:%s\r\n", cJSON_GetStringValue(city));
            printf("%s, %s, %s\r\n", cJSON_GetStringValue(text), cJSON_GetStringValue(code), cJSON_GetStringValue(temp));

            // 检查各数据是否成功获取
            if (city != NULL && text != NULL && code != NULL && temp != NULL)
            {
                strcpy(weather_data.location, cJSON_GetStringValue(city));
                strcpy(weather_data.text, cJSON_GetStringValue(text));

                weather_data.code = atoi(cJSON_GetStringValue(code));
                weather_data.temp = atoi(cJSON_GetStringValue(temp));

                return &weather_data;
            }

            cJSON_Delete(root);
        }
    }

    return NULL;
}
