#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_http_server.h"
#include "esp_sntp.h"
#include "app_state.h"

#include "network_config.h"

#define TAG "Threadnet App"
#define MAX_NODES 16
#define MAX_CONTENT_LENGTH 1024
#define NODE_TIMEOUT 60

threadnet_packet_t threadnet_packet[MAX_NODES];

// Header for CSV response
static char CSV_HEADER[] = "index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate\n";

// Handler for the root path
esp_err_t get_handler(httpd_req_t *req)
{
    char resp[MAX_CONTENT_LENGTH];
    uint32_t cursor = 0;

    // Get the current time
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    // Write the CSV header
    cursor += snprintf(resp + cursor, MAX_CONTENT_LENGTH - cursor, CSV_HEADER);

    // Iterate through the tracked nodes and write their status to the response
    for (int i = 0; i < MAX_NODES; i++)
    {
        threadnet_packet_t *packet = &threadnet_packet[i];

        // Check if the node has timed out
        if (tv_now.tv_sec - packet->last_time > NODE_TIMEOUT)
        {
            packet->ot_rloc_16 = 0;
        }

        // Write the node status to the response if it is valid
        if (packet->ot_rloc_16 != 0)
        {
            cursor += snprintf(resp + cursor, MAX_CONTENT_LENGTH - cursor, "%d,%d,%s,%s,%d,%d,%lu,%d,%d,%d,%llu,%d,%d,%d,%d,%d,%d,%ld,%.2f\n", i, packet->role, packet->ot_mesh_local_eid, packet->ot_link_local_ipv6, packet->ot_leader_id, packet->ot_leader_weight, packet->ot_partition_id, packet->ot_rloc_16, packet->parent_avg_rssi, packet->parent_last_rssi, tv_now.tv_sec - packet->last_time, packet->parent_next_hop, packet->parent_link_quality_in, packet->parent_link_quality_out, packet->parent_rloc16, packet->parent_age, packet->parent_router_id, packet->avg_rtt_millis, packet->packet_success_rate);
        }
    }

    // Set Content Type header
    httpd_resp_set_type(req, "text/csv");

    // Send the response
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for changing the transport mode
esp_err_t transport_set_handler(httpd_req_t *req)
{
    // Get the content length
    size_t content_length = req->content_len;
    char *content = malloc(content_length + 1);
    if (content == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for content");
        return ESP_FAIL;
    }

    // Read the request data
    if (httpd_req_recv(req, content, content_length) <= 0)
    {
        ESP_LOGE(TAG, "Failed to read content");
        free(content);
        return ESP_FAIL;
    }
    content[content_length] = '\0';

    // parse the content (check if it is "udp", "tcp", or "multi")
    if (strcmp(content, "UDP") == 0)
    {
        set_transport_mode(TRANSPORT_MODE_UDP);
    }
    else if (strcmp(content, "TCP") == 0)
    {
        set_transport_mode(TRANSPORT_MODE_TCP);
    }
    else if (strcmp(content, "MULTI") == 0)
    {
        set_transport_mode(TRANSPORT_MODE_MULTI);
    }
    else
    {
        ESP_LOGI(TAG, "Invalid transport mode: %s", content);
        free(content);

        /* Send a simple response */
        char resp[] = "INVALID\n";
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

        return ESP_OK;
    }

    free(content);

    // Send response
    char resp[] = "OK\n";
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for getting the transport mode
esp_err_t transport_get_handler(httpd_req_t *req)
{
    char resp[MAX_CONTENT_LENGTH];

    // Set the response based on the current transport mode
    switch (get_transport_mode())
    {
    case TRANSPORT_MODE_UDP:
        snprintf(resp, MAX_CONTENT_LENGTH, "udp");
        break;
    case TRANSPORT_MODE_TCP:
        snprintf(resp, MAX_CONTENT_LENGTH, "tcp");
        break;
    case TRANSPORT_MODE_MULTI:
        snprintf(resp, MAX_CONTENT_LENGTH, "multi");
        break;
    default:
        snprintf(resp, MAX_CONTENT_LENGTH, "INVALID");
        break;
    }

    // Send the response
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler definitions
httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_change_transport = {
    .uri = "/transport",
    .method = HTTP_POST,
    .handler = transport_set_handler,
    .user_ctx = NULL};

httpd_uri_t uri_get_transport = {
    .uri = "/transport",
    .method = HTTP_GET,
    .handler = transport_get_handler,
    .user_ctx = NULL};

// Start the threadnet app
void start_threadnet_app(void)
{
    ESP_LOGI(TAG, "Starting Threadnet App");

    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP server started");

        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_change_transport);
        httpd_register_uri_handler(server, &uri_get_transport);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}

// Update the tracked node based on the received packet
uint8_t update_tracked_node(threadnet_packet_t *packet)
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (threadnet_packet[i].ot_rloc_16 == 0 || threadnet_packet[i].ot_rloc_16 == packet->ot_rloc_16)
        {
            threadnet_packet[i] = *packet;
            return 0;
        }
    }
    return 1;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}