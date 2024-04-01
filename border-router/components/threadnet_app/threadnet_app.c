#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_http_server.h"

#define TAG "Threadnet App"

esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char resp[] = "URI GET Response";
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

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}