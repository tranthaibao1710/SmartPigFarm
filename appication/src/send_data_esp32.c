/*
 * send_data_esp32_simple.c
 *
 *  Gửi dữ liệu cảm biến từ STM32 sang ESP32 qua UART
 */

#include "send_data_esp32.h"
#include "uart.h"

#define ESP32_UART          USART1      // UART dùng để giao tiếp với ESP32
#define JSON_BUFFER_SIZE    256
#define UART_TX_BUFFER_SIZE 300

static char json_buffer[JSON_BUFFER_SIZE];
static char uart_tx_buffer[UART_TX_BUFFER_SIZE];
static uint8_t packet_counter = 0;

/**
 * @brief Tạo chuỗi JSON từ dữ liệu cảm biến
 */
  uint8_t gio, phut, giay,thu, ngay, thang, nam;
  
void CreateJSONString(void) {
 
  DS3231Read(&gio, &phut, &giay, &thu, &ngay, &thang, &nam);
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u",
             gio, phut, giay);
    snprintf(json_buffer, JSON_BUFFER_SIZE,
        "{"
            "\"id\":%d,"
            "\"time\":\"%s\","
            "\"nh3\":%.2f,"
            "\"h2s\":%.2f"
        "}",
        packet_counter,
        time_str,
        GetNH3_PPM(),
        GetH2S_PPM()
    );
}

/**
 * @brief Gửi dữ liệu JSON sang ESP32 qua UART
 */
void SendDataToESP32(void) {
    // Tạo dữ liệu JSON
    CreateJSONString();

    // Đóng gói dữ liệu (thêm header và footer để ESP32 dễ nhận)
    snprintf(uart_tx_buffer, UART_TX_BUFFER_SIZE,
             "START:%s:END\r\n", json_buffer);

    // Gửi dữ liệu qua UART
    USART_PutS(ESP32_UART, uart_tx_buffer);

    // Log để kiểm tra
    printf("✅ Gửi data ESP32 (ID:%d)\r\n", packet_counter);

    packet_counter++;
}
