#include "captive_portal.h"

#define DNS_PORT 53
#define DNS_MAX_PACKET_SIZE 512
static const char *TAG = "CAPTIVE_PORTAL";

static int captive_portal_build_dns_response(const uint8_t *request, int request_len, uint8_t *response,
                                             const char *redirect_ip) {
    if (request_len < 12) {
        return 0;
    }

    memcpy(response, request, request_len);

    // FAUDRAIT QUE JE REPRENNE CA J'AI RIEN PIGE

    // Set response flags: QR (response), Opcode (0), AA (authoritative), TC (0), RD (copied from request)
    response[2] = 0x81;  // QR = 1, AA = 1
    response[3] = 0x80;  // RA = 1, all other flags 0

    // Set question count to 1 (copied from request)
    // Set answer count to 1
    response[6] = 0x00;
    response[7] = 0x01;

    // Copy the question section from the request
    int pos = request_len;

    // Add the answer section
    response[pos++] = 0xC0;  // Pointer to the domain name in the question section
    response[pos++] = 0x0C;

    // Type A (host address)
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // Class IN (Internet)
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // TTL (Time to live) - 4 bytes
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x3C;  // 60 seconds

    // Data length - 2 bytes
    response[pos++] = 0x00;
    response[pos++] = 0x04;  // IPv4 address is 4 bytes

    // IP address (redirect_ip)
    uint8_t ip[4];
    // Extract the queried domain name from the DNS request
    char domain_name[256] = {0};
    int query_pos = 12;  // DNS header is 12 bytes
    int domain_pos = 0;

    // FAUDRAIT QUE JE REPRENNE CA J'AI RIEN PIGE

    while (query_pos < request_len && request[query_pos] != 0) {
        uint8_t label_length = request[query_pos++];
        if (label_length + query_pos > request_len || domain_pos + label_length >= sizeof(domain_name) - 1) {
            ESP_LOGE(TAG, "Invalid domain name in DNS request");
            return 0;
        }
        memcpy(&domain_name[domain_pos], &request[query_pos], label_length);
        domain_pos += label_length;
        query_pos += label_length;
        domain_name[domain_pos++] = '.';  // Add dot between labels
    }

    if (domain_pos > 0) {
        domain_name[domain_pos - 1] = '\0';  // Replace the last dot with null terminator
    }

    ESP_LOGI(TAG, "Queried domain: %s", domain_name);
    sscanf(redirect_ip, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]);
    response[pos++] = ip[0];
    response[pos++] = ip[1];
    response[pos++] = ip[2];
    response[pos++] = ip[3];

    return pos;
}

static void captive_portal_dns_server_task(void *pvParameters) {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buf[DNS_MAX_PACKET_SIZE];
    uint8_t response[DNS_MAX_PACKET_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Error on socket creation, code : %d", errno);
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DNS_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Error on socket bind, code %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "DNS server listening on %d", DNS_PORT);

    while (1) {
        int len = recvfrom(sock, buf, DNS_MAX_PACKET_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) {
            ESP_LOGE(TAG, "Error recvfrom, code %d", errno);
            break;
        }

        ESP_LOGI(TAG, "DNS request received, length = %d", len);

        int resp_len = captive_portal_build_dns_response(buf, len, response, "10.0.1.1");

        if (sendto(sock, response, resp_len, 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
            ESP_LOGE(TAG, "Erreur sendto, code %d", errno);
        }
    }

    close(sock);
    vTaskDelete(NULL);
}

void captive_portal_start_dns_server(void) {
    xTaskCreate(captive_portal_dns_server_task, "dns_server", 4096, NULL, 5, NULL);
}
