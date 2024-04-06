#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_http_server.h"
#include "esp_sntp.h"

#include "network_config.h"

#define TAG "Threadnet App"
#define MAX_NODES 16
#define MAX_CONTENT_LENGTH 1024
#define NODE_TIMEOUT 60

threadnet_packet_t threadnet_packet[MAX_NODES];

static char CSV_HEADER[] = "index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate\n";

esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    char resp[MAX_CONTENT_LENGTH];
    uint32_t cursor = 0;

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    cursor += snprintf(resp + cursor, MAX_CONTENT_LENGTH - cursor, CSV_HEADER);
    for (int i = 0; i < MAX_NODES; i++)
    {
        threadnet_packet_t *packet = &threadnet_packet[i];

        if (tv_now.tv_sec - packet->last_time > NODE_TIMEOUT)
        {
            packet->ot_rloc_16 = 0;
        }

        if (packet->ot_rloc_16 != 0)
        {
            cursor += snprintf(resp + cursor, MAX_CONTENT_LENGTH - cursor, "%d,%d,%s,%s,%d,%d,%lu,%d,%d,%d,%llu,%d,%d,%d,%d,%d,%d,%ld,%.2f\n", i, packet->role, packet->ot_mesh_local_eid, packet->ot_link_local_ipv6, packet->ot_leader_id, packet->ot_leader_weight, packet->ot_partition_id, packet->ot_rloc_16, packet->parent_avg_rssi, packet->parent_last_rssi, tv_now.tv_sec - packet->last_time, packet->parent_next_hop, packet->parent_link_quality_in, packet->parent_link_quality_out, packet->parent_rloc16, packet->parent_age, packet->parent_router_id, packet->avg_rtt_millis, packet->packet_success_rate);
        }
    }

    httpd_resp_set_type(req, "text/csv");

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

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
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}

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