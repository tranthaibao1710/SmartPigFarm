/*
 * send_data_esp32.c
 *
 *  Created on: Jul 29, 2025
 *      Author: FPTSHOP
 */
#include "send_data_esp32.h"
/* =============================================================================
 * MODULE UART ESP32 - SỬ DỤNG HÀM UART TÙY CHỈNH
 * Tích hợp với uart.c đã có sẵn
 * =============================================================================
 */

#include "uart.h"

// =============================================================================
// UART ESP32 CONFIGURATION
// =============================================================================
#define ESP32_UART              USART1      // Sử dụng USART1 cho ESP32
#define ESP32_UART_PIN          PA9PA10     // PA9=TX, PA10=RX
#define ESP32_BAUDRATE          115200      // Baudrate

// Buffer sizes
#define JSON_BUFFER_SIZE        512
#define UART_TX_BUFFER_SIZE     600
#define UART_RX_BUFFER_SIZE     256

// =============================================================================
// UART ESP32 VARIABLES
// =============================================================================
static uint8_t packet_counter = 0;
static char json_buffer[JSON_BUFFER_SIZE];
static char uart_tx_buffer[UART_TX_BUFFER_SIZE];
static char uart_rx_buffer[UART_RX_BUFFER_SIZE];

// Timing variables
static uint32_t last_data_send = 0;
static uint32_t last_heartbeat = 0;

// Status variables
static uint8_t esp32_connected = 0;
static uint32_t esp32_last_response = 0;

// =============================================================================
// UART ESP32 FUNCTIONS
// =============================================================================

/**
 * @brief Khởi tạo UART cho ESP32
 */


/**
 * @brief Tạo chuỗi JSON từ dữ liệu cảm biến
 */
void CreateJSONString(void) {
    const char* alarm_levels[] = {"NORMAL", "LOW", "HIGH", "DANGER"};
    const char* status_text[] = {"ERROR", "BASIC", "GOOD", "OPTIMAL"};
    
    // Lấy dữ liệu hiện tại
    float nh3_ppm = GetNH3_PPM();
    float h2s_ppm = GetH2S_PPM();
    uint8_t nh3_alarm = (uint8_t)GetNH3AlarmLevel();
    uint8_t h2s_alarm = (uint8_t)GetH2SAlarmLevel();
    uint8_t sys_alarm = (uint8_t)GetSystemAlarmLevel();
    uint8_t sys_status = GetSystemStatus();
    
    // Tạo JSON string
    snprintf(json_buffer, JSON_BUFFER_SIZE,
        "{"
        "\"id\":%d,"
        "\"time\":%lu,"
        "\"nh3\":{"
            "\"ppm\":%.1f,"
            "\"alarm\":\"%s\","
            "\"voltage\":%.3f,"
            "\"resistance\":%.1f,"
            "\"rs_r0\":%.3f,"
            "\"digital\":%d,"
            "\"valid\":%d"
        "},"
        "\"h2s\":{"
            "\"ppm\":%.1f,"
            "\"alarm\":\"%s\","
            "\"voltage\":%.3f,"
            "\"resistance\":%.1f,"
            "\"rs_r0\":%.3f,"
            "\"digital\":%d,"
            "\"valid\":%d"
        "},"
        "\"system\":{"
            "\"status\":\"%s\","
            "\"alarm\":\"%s\","
            "\"uptime\":%lu,"
            "\"readings\":%lu,"
            "\"errors\":%lu"
        "}"
        "}",
        packet_counter,
        HAL_GetTick() / 1000,
        
        // NH3 data
        nh3_ppm,
        alarm_levels[nh3_alarm],
        g_sensor_system.mq137.base.raw_voltage,
        g_sensor_system.mq137.base.resistance,
        g_sensor_system.mq137.base.rs_r0_ratio,
        g_sensor_system.mq137.base.digital_state,
        g_sensor_system.mq137.base.is_valid,
        
        // CO2 data
        h2s_ppm,
        alarm_levels[h2s_alarm],
        g_sensor_system.mq136.base.raw_voltage,
        g_sensor_system.mq136.base.resistance,
        g_sensor_system.mq136.base.rs_r0_ratio,
        g_sensor_system.mq136.base.digital_state,
        g_sensor_system.mq136.base.is_valid,
        
        // System data
        status_text[sys_status],
        alarm_levels[sys_alarm],
        HAL_GetTick() / 1000,
        g_sensor_system.total_readings,
        g_sensor_system.error_count
    );
}

/**
 * @brief Gửi dữ liệu JSON qua UART đến ESP32
 */
uint8_t SendJSONData(void) {
    // Tạo frame với header và footer
    snprintf(uart_tx_buffer, UART_TX_BUFFER_SIZE, 
             "START_JSON:%s:END_JSON\r\n", json_buffer);
    
    // Gửi qua UART tùy chỉnh
    USART_PutS(ESP32_UART, uart_tx_buffer);
    
    return 1; // Success
}

/**
 * @brief Gửi lệnh điều khiển đến ESP32
 */
void SendCommandToESP32(const char* command) {
    snprintf(uart_tx_buffer, UART_TX_BUFFER_SIZE, 
             "CMD:%s\r\n", command);
    
    USART_PutS(ESP32_UART, uart_tx_buffer);
    
    printf("📤 ESP32 << %s\r\n", command);
}

/**
 * @brief Đọc phản hồi từ ESP32
 */
uint16_t ReadESP32Response(void) {
    uint16_t bytes_read = 0;
    
    // Đọc dữ liệu từ buffer UART
    bytes_read = USARTx_GetS(ESP32_UART, uart_rx_buffer, UART_RX_BUFFER_SIZE);
    
    if(bytes_read > 0) {
        uart_rx_buffer[bytes_read] = '\0'; // Null terminate
        
        // Xử lý response
        ProcessESP32Response(uart_rx_buffer);
        
        // Cập nhật thời gian response cuối
        esp32_last_response = HAL_GetTick();
    }
    
    return bytes_read;
}

/**
 * @brief Xử lý phản hồi từ ESP32
 */
void ProcessESP32Response(char* response) {
    printf("📥 ESP32 >> %s", response); // response đã có \r\n
    
    // Kiểm tra các response cụ thể
    if(strstr(response, "PONG")) {
        esp32_connected = 1;
        printf("✓ ESP32 đã kết nối!\r\n");
    }
    else if(strstr(response, "ESP32_READY")) {
        esp32_connected = 1;
        printf("✓ ESP32 sẵn sàng nhận dữ liệu!\r\n");
    }
    else if(strstr(response, "DATA_OK")) {
        printf("✓ ESP32 đã nhận dữ liệu JSON\r\n");
    }
    else if(strstr(response, "ERROR")) {
        printf("❌ ESP32 báo lỗi: %s", response);
    }
    else if(strstr(response, "WIFI_CONNECTED")) {
        printf("✓ ESP32 đã kết nối WiFi\r\n");
    }
    else if(strstr(response, "BLYNK_CONNECTED")) {
        printf("✓ ESP32 đã kết nối Blynk\r\n");
    }
}

/**
 * @brief Gửi dữ liệu chính đến ESP32
 */
void SendDataToESP32(void) {
    // Tạo JSON từ dữ liệu hiện tại
    CreateJSONString();
    
    // Gửi JSON data
    if(SendJSONData()) {
        printf("✓ Gửi data ESP32 (ID:%d, Size:%d bytes)\r\n", 
               packet_counter, strlen(json_buffer));
        packet_counter++;
        last_data_send = HAL_GetTick();
    } else {
        printf("❌ Lỗi gửi data ESP32\r\n");
    }
}

/**
 * @brief Gửi cảnh báo khi alarm thay đổi
 */
void SendAlarmUpdate(void) {
    static AlarmLevel_t last_system_alarm = ALARM_NORMAL;
    static AlarmLevel_t last_nh3_alarm = ALARM_NORMAL;
    static AlarmLevel_t last_h2s_alarm = ALARM_NORMAL;
    
    AlarmLevel_t current_system = GetSystemAlarmLevel();
    AlarmLevel_t current_nh3 = GetNH3AlarmLevel();
    AlarmLevel_t current_h2s = GetH2SAlarmLevel();
    
    // System alarm changed
    if(current_system != last_system_alarm) {
        switch(current_system) {
            case ALARM_DANGER:
                SendCommandToESP32("ALERT_DANGER");
                break;
            case ALARM_HIGH:
                SendCommandToESP32("ALERT_HIGH");
                break;
            case ALARM_LOW:
                SendCommandToESP32("ALERT_LOW");
                break;
            default:
                SendCommandToESP32("ALERT_NORMAL");
                break;
        }
        last_system_alarm = current_system;
    }
    
    // NH3 alarm changed
    if(current_nh3 != last_nh3_alarm) {
        if(current_nh3 >= ALARM_HIGH) {
            SendCommandToESP32("NH3_HIGH");
        } else {
            SendCommandToESP32("NH3_NORMAL");
        }
        last_nh3_alarm = current_nh3;
    }
    
    // CO2 alarm changed
    if(current_h2s != last_h2s_alarm) {
        if(current_h2s >= ALARM_HIGH) {
            SendCommandToESP32("H2S_HIGH");
        } else {
            SendCommandToESP32("H2S_NORMAL");
        }
        last_h2s_alarm = current_h2s;
    }
}

/**
 * @brief Gửi heartbeat để kiểm tra kết nối
 */


/**
 * @brief Test giao tiếp ESP32
 */


/**
 * @brief Main loop đã tích hợp ESP32
 */
void UpdateMainLoopWithESP32(void) {
    static uint32_t last_display_update = 0;
    static uint32_t last_esp32_check = 0;
    
    uint32_t current_time = HAL_GetTick();
    
    // 1. Xử lý cảm biến (luôn chạy)
    ProcessAllSensors();
    
    // 2. Đọc response từ ESP32
    ReadESP32Response();
    
    // 3. Gửi alarm ngay khi thay đổi
    SendAlarmUpdate();
    
    // 4. Hiển thị local mỗi 5 giây
    if(current_time - last_display_update >= 5000) {
        DisplaySystemStatus();
        last_display_update = current_time;
    }
    
    // 5. Gửi dữ liệu đến ESP32 mỗi 10 giây
    if(current_time - last_data_send >= 10000) {
        SendDataToESP32();
    }
    
    // 6. Kiểm tra kết nối ESP32 mỗi 5 giây
    if(current_time - last_esp32_check >= 5000) {
        CheckESP32Connection();
        SendHeartbeat();
        last_esp32_check = current_time;
    }
    
    // 7. Logic điều khiển thiết bị
    HandleDeviceControl();
    
    // 8. Delay nhỏ
    HAL_Delay(1000);
}

/**
 * @brief Xử lý điều khiển thiết bị dựa trên cảnh báo
 */
void HandleDeviceControl(void) {
    static uint32_t last_control_log = 0;
    uint32_t current_time = HAL_GetTick();
    
    AlarmLevel_t system_alarm = GetSystemAlarmLevel();
    AlarmLevel_t nh3_alarm = GetNH3AlarmLevel();
    AlarmLevel_t h2s_alarm = GetH2SAlarmLevel();
    
    // In log điều khiển mỗi 30 giây
    uint8_t should_log = (current_time - last_control_log >= 30000);
    
    // Điều khiển quạt NH3
    if(nh3_alarm >= ALARM_HIGH) {
        if(should_log) {
            printf("🚨 NH3 = %.1f ppm - Bật quạt thông gió!\r\n", GetNH3_PPM());
        }
        SendCommandToESP32("FAN_NH3_ON");
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_SET);
    } else if(nh3_alarm <= ALARM_NORMAL) {
        SendCommandToESP32("FAN_NH3_OFF");
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_RESET);
    }
    
    // Điều khiển quạt H2S
    if(h2s_alarm >= ALARM_HIGH) {
        if(should_log) {
            printf("🚨 H2S = %.1f ppm - Bật quạt thông gió!\r\n", GetH2S_PPM());
        }
        SendCommandToESP32("FAN_H2S_ON");
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_SET);
    } else if(h2s_alarm <= ALARM_NORMAL) {
        SendCommandToESP32("FAN_H2S_OFF");
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_RESET);
    }
    
    // Buzzer cảnh báo
    if(system_alarm >= ALARM_DANGER) {
        if(should_log) {
            printf("🚨🚨 NGUY HIỂM! BẬT BUZZER! 🚨🚨\r\n");
        }
        SendCommandToESP32("BUZZER_ON");
        // HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
    } else {
        SendCommandToESP32("BUZZER_OFF");
        // HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
    }
    
    if(should_log) {
        last_control_log = current_time;
        printf("📊 Status: ESP32=%s, NH3=%.1f, CO2=%.1f\r\n",
               esp32_connected ? "OK" : "FAIL", GetNH3_PPM(), GetH2S_PPM());
    }
}

// =============================================================================
// HƯỚNG DẪN TÍCH HỢP
// =============================================================================

/*
 * 1. THÊM VÀO main.c:
 * 
 * #include "uart.h"  // Thêm include
 * 
 * int main(void) {
 *     HAL_Init();
 *     SystemClock_Config();
 *     MX_GPIO_Init();
 *     MX_ADC1_Init();
 *     // KHÔNG CẦN MX_USART1_UART_Init() - đã có USARTx_Init()
 *     
 *     InitSensorSystem();
 *     InitESP32Communication();  // THÊM DÒNG NÀY
 *     
 *     TestESP32Communication();  // Tùy chọn
 *     
 *     printf("🚀 BẮT ĐẦU GIÁM SÁT\r\n");
 *     
 *     while(1) {
 *         UpdateMainLoopWithESP32();  // THAY THẾ main loop cũ
 *     }
 * }
 * 
 * 2. FILE STRUCTURE:
 *    - main.c: Code cảm biến chính + tích hợp ESP32
 *    - uart.c: Hàm UART tùy chỉnh (đã có)
 *    - uart.h: Header UART (cần thêm khai báo)
 * 
 * 3. UART.H CẦN THÊM:
 *    - Khai báo các hàm: USARTx_Init, USART_PutS, USARTx_GetS, v.v.
 *    - Include cần thiết: main.h, stdio.h
 * 
 * 4. KẾT NỐI:
 *    STM32 PA9 (TX)  → ESP32 GPIO16 (RX)
 *    STM32 PA10 (RX) → ESP32 GPIO17 (TX)
 *    GND             → GND
 * 
 * 5. DEBUG:
 *    - Mở Serial Monitor ở 115200 baud
 *    - Quan sát log: "✓ Gửi data ESP32"
 *    - Kiểm tra ESP32 response: "📥 ESP32 >>"
 */
