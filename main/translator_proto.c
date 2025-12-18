#include "translator_proto.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lwip/inet.h"

uint64_t translator_htonll(uint64_t v) {
    uint32_t hi = htonl((uint32_t)(v >> 32));
    uint32_t lo = htonl((uint32_t)(v & 0xffffffffu));
    return ((uint64_t)lo << 32) | hi;
}

uint64_t translator_ntohll(uint64_t v) {
    uint32_t hi = ntohl((uint32_t)(v >> 32));
    uint32_t lo = ntohl((uint32_t)(v & 0xffffffffu));
    return ((uint64_t)lo << 32) | hi;
}

static bool send_exact(int sock, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    while (len > 0) {
        ssize_t n = send(sock, p, len, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false;
        }
        p += (size_t)n;
        len -= (size_t)n;
    }
    return true;
}

bool translator_recv_exact(int sock, void *buf, size_t len) {
    uint8_t *p = (uint8_t *)buf;
    while (len > 0) {
        ssize_t n = recv(sock, p, len, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false;
        }
        p += (size_t)n;
        len -= (size_t)n;
    }
    return true;
}

bool translator_send_frame(int sock,
                           translator_msg_type_t msg_type,
                           uint32_t seq,
                           uint64_t timestamp_us,
                           const void *payload,
                           uint32_t payload_len) {
    translator_frame_header_t hdr = {
        .magic = htonl(TRANSLATOR_MAGIC),
        .version = htons((uint16_t)TRANSLATOR_PROTO_VERSION),
        .msg_type = htons((uint16_t)msg_type),
        .payload_len = htonl(payload_len),
        .seq = htonl(seq),
        .timestamp_us = translator_htonll(timestamp_us),
    };

    if (!send_exact(sock, &hdr, sizeof(hdr))) {
        return false;
    }
    if (payload_len == 0) {
        return true;
    }
    if (payload == NULL) {
        return false;
    }
    return send_exact(sock, payload, payload_len);
}

bool translator_recv_frame_header(int sock, translator_frame_header_t *out_hdr) {
    translator_frame_header_t hdr;
    if (!translator_recv_exact(sock, &hdr, sizeof(hdr))) {
        return false;
    }

    if (ntohl(hdr.magic) != TRANSLATOR_MAGIC) {
        errno = EPROTO;
        return false;
    }
    if (ntohs(hdr.version) != TRANSLATOR_PROTO_VERSION) {
        errno = EPROTO;
        return false;
    }

    if (out_hdr != NULL) {
        *out_hdr = hdr;
    }
    return true;
}
