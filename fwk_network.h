// network framework
// - rlyeh, public domain

int download(FILE *out, const char *url);

// int send_game_state(void *ptr, int len, int flags); PROTOCOL_V1|QUANTIZE|COMPRESS|RLE
// int recv_game_state(); compensate, extrapolate, intrapolate(); lerp();

#ifdef NETWORK_C
#pragma once

#if defined(_WIN32)
#   include <winsock2.h>
#   include <wininet.h>
#   pragma comment(lib,"wininet.lib")
#endif

int download(FILE *out, const char *url) {
#ifdef _WIN32
    DWORD ok = 0, lBytesRead = 0;
    HINTERNET session = InternetOpenA("request 1", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
    if( session ) {
        HINTERNET request = InternetOpenUrlA(session, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if( request ) {
            enum { buflen = 4096 };
            char buffer[ buflen ];
            for(; (ok = (InternetReadFile(request, buffer, buflen, &lBytesRead) != FALSE) && lBytesRead > 0); ) {
                ok = 1 == fwrite(buffer, lBytesRead, 1, out);
                if( !ok ) break;
            }
            InternetCloseHandle(request);
        }
        InternetCloseHandle(session);
    }
    return ok && !lBytesRead;
#else
    return 0;
#endif
}

#endif // DOWNLOAD_C
