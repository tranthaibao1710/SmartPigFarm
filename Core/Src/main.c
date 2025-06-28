/* =============================================================================
 * HỆ THỐNG GIÁM SÁT KHÍ MQ137 & MQ135 - MODULAR DESIGN
 * Phiên bản: 3.0 - Thiết kế modular với 2 cảm biến
 * =============================================================================
 */

#include "main.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

// =============================================================================
// CẤU HÌNH PHẦN CỨNG
// =============================================================================
/*
 * Sơ đồ kết nối:
 * MQ137 (NH3)          →    STM32F103        MQ135 (CO2)        →    STM32F103
 * -----------               ---------        -----------------        ---------
 * VCC                  →    5V              VCC                 →    5V
 * GND                  →    GND             GND                 →    GND
 * AOUT                 →    PA0 (ADC1_CH0)  AOUT                →    PA1 (ADC1_CH1)
 * DOUT                 →    PA3 (GPIO)      DOUT                →    PA4 (GPIO)
 *
 * LƯU Ý: Cần mạch chia áp cho AOUT vì STM32F103 chỉ đo được 0-3.3V
 * Mạch chia áp: AOUT → R1(2.7kΩ) → ADC_PIN → R2(5.1kΩ) → GND
 * Tỷ lệ chia: 3.3V / 5V = 0.66
 */

// Cấu hình chân
#define MQ137_ADC_CHANNEL ADC_CHANNEL_0
#define MQ137_DIGITAL_PORT GPIOA
#define MQ137_DIGITAL_PIN GPIO_PIN_3

#define MQ135_ADC_CHANNEL ADC_CHANNEL_1
#define MQ135_DIGITAL_PORT GPIOA
#define MQ135_DIGITAL_PIN GPIO_PIN_4

// Cấu hình ADC và mạch chia áp
#define ADC_VREF 3.3f               // Điện áp tham chiếu ADC (V)
#define ADC_RESOLUTION 4095         // 12-bit ADC
#define SENSOR_VCC 5.0f             // Điện áp cấp cho cảm biến (V)
#define VOLTAGE_DIVIDER_RATIO 0.66f // Tỷ lệ chia áp (3.3V/5V)
#define LOAD_RESISTANCE 10000.0f    // Điện trở tải (Ω)

// =============================================================================
// ENUM VÀ ĐỊNH NGHĨA
// =============================================================================
typedef enum
{
  SENSOR_TYPE_MQ137 = 0, // NH3 Sensor
  SENSOR_TYPE_MQ135      // Multi-gas Sensor (CO2)
} SensorType_t;

typedef enum
{
  GAS_TYPE_NH3 = 0,
  GAS_TYPE_CO2
} GasType_t;

typedef enum
{
  ALARM_NORMAL = 0, // Bình thường
  ALARM_LOW,        // Thấp
  ALARM_HIGH,       // Cao
  ALARM_DANGER      // Nguy hiểm
} AlarmLevel_t;

// =============================================================================
// CẤU TRÚC CẢM BIẾN CƠ BẢN
// =============================================================================
typedef struct
{
  // Thông tin định danh
  uint8_t sensor_id;        // ID cảm biến (0, 1, 2...)
  SensorType_t sensor_type; // Loại cảm biến
  char sensor_name[16];     // Tên cảm biến
  uint8_t is_enabled;       // Cảm biến có hoạt động không
  uint8_t is_valid;         // Dữ liệu có hợp lệ không

  // Cấu hình phần cứng
  uint32_t adc_channel;       // Kênh ADC
  GPIO_TypeDef *digital_port; // Port GPIO digital
  uint16_t digital_pin;       // Pin GPIO digital

  // Dữ liệu đo thô
  float raw_voltage;     // Điện áp ADC (V)
  float resistance;      // Điện trở cảm biến (Ω)
  float rs_r0_ratio;     // Tỷ lệ Rs/R0
  uint8_t digital_state; // Trạng thái digital output
  uint32_t timestamp;    // Thời gian đo (ms)

  // Thông số hiệu chuẩn
  float r0_value; // Giá trị R0 (hiệu chuẩn)
} BaseSensor_t;

// =============================================================================
// CẤU TRÚC CẢM BIẾN KHÍ (KẾ THỪA TỪ BASE SENSOR)
// =============================================================================
typedef struct
{
  BaseSensor_t base; // Cảm biến cơ bản

  // Thông tin khí đo
  GasType_t gas_type; // Loại khí (NH3, CO2...)
  char gas_name[8];   // Tên khí ("NH3", "CO2")
  char unit[8];       // Đơn vị ("ppm")

  // Giá trị nồng độ
  float gas_ppm;      // Nồng độ khí hiện tại (ppm)
  float filtered_ppm; // Giá trị sau lọc nhiễu
  float average_ppm;  // Giá trị trung bình

  // Ngưỡng cảnh báo
  float threshold_low;      // Ngưỡng cảnh báo thấp
  float threshold_high;     // Ngưỡng cảnh báo cao
  float threshold_danger;   // Ngưỡng nguy hiểm
  AlarmLevel_t alarm_level; // Mức cảnh báo hiện tại

  // Thông số đặc tuyến cảm biến
  float curve_a; // Hệ số A trong công thức ppm = A * (Rs/R0)^B
  float curve_b; // Hệ số B
  float min_ppm; // Giá trị ppm tối thiểu
  float max_ppm; // Giá trị ppm tối đa
} GasSensor_t;

// =============================================================================
// HỆ THỐNG QUẢN LÝ CẢM BIẾN
// =============================================================================
typedef struct
{
  // Cảm biến cụ thể
  GasSensor_t mq137; // Cảm biến MQ137 (NH3)
  GasSensor_t mq135; // Cảm biến MQ135 (CO2)

  // Trạng thái hệ thống
  uint8_t active_sensor_count; // Số cảm biến đang hoạt động
  uint8_t system_status;       // Trạng thái hệ thống (0=lỗi, 1=cơ bản, 2=tốt, 3=tối ưu)
  AlarmLevel_t system_alarm;   // Mức cảnh báo tổng thể
  uint32_t last_update;        // Thời gian cập nhật cuối

  // Thống kê
  uint32_t total_readings; // Tổng số lần đo
  uint32_t error_count;    // Số lần đo lỗi
  float uptime_minutes;    // Thời gian hoạt động (phút)
} SensorSystem_t;

// =============================================================================
// BIẾN TOÀN CỤC
// =============================================================================
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

SensorSystem_t g_sensor_system;

// Buffer lưu lịch sử
#define HISTORY_SIZE 60
SensorSystem_t history_buffer[HISTORY_SIZE];
uint8_t history_index = 0;

// =============================================================================
// KHAI BÁO HÀM
// =============================================================================
// Hàm khởi tạo và quản lý hệ thống
void InitSensorSystem(void);
void ProcessAllSensors(void);
void DisplaySystemStatus(void);
void TestSensorSystem(void);
void CalibrateSensors(void);

// Hàm xử lý cảm biến riêng lẻ
void InitMQ137Sensor(GasSensor_t *sensor);
void InitMQ135Sensor(GasSensor_t *sensor);
void ProcessGasSensor(GasSensor_t *sensor);
uint8_t ValidateGasSensor(GasSensor_t *sensor);
void UpdateAlarmLevel(GasSensor_t *sensor);

// Hàm đọc dữ liệu phần cứng
float ReadADC_Voltage(uint32_t channel);
uint8_t ReadDigitalState(GPIO_TypeDef *port, uint16_t pin);

// Hàm tính toán
float CalculateResistance(float voltage);
float CalculateGasPPM(float rs_r0_ratio, float curve_a, float curve_b);
AlarmLevel_t DetermineAlarmLevel(float ppm, float low, float high, float danger);

// Hàm tiện ích
void SaveToHistory(void);
void PrintSensorDetails(GasSensor_t *sensor);
void UpdateSystemStatus(void);

// Getter functions
float GetNH3_PPM(void);
float GetCO2_PPM(void);
AlarmLevel_t GetNH3AlarmLevel(void);
AlarmLevel_t GetCO2AlarmLevel(void);
AlarmLevel_t GetSystemAlarmLevel(void);
uint8_t GetSystemStatus(void);

// =============================================================================
// TRIỂN KHAI HÀM
// =============================================================================

/**
 * @brief Khởi tạo hệ thống cảm biến
 */
void InitSensorSystem(void)
{
  printf("\r\n=== KHỞI TẠO HỆ THỐNG CẢM BIẾN ===\r\n");

  // Reset toàn bộ hệ thống
  memset(&g_sensor_system, 0, sizeof(SensorSystem_t));
  memset(history_buffer, 0, sizeof(history_buffer));

  // Khởi tạo từng cảm biến
  InitMQ137Sensor(&g_sensor_system.mq137);
  InitMQ135Sensor(&g_sensor_system.mq135);

  // Cấu hình GPIO cho digital pins
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // MQ137 Digital Pin
  GPIO_InitStruct.Pin = MQ137_DIGITAL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MQ137_DIGITAL_PORT, &GPIO_InitStruct);

  // MQ135 Digital Pin
  GPIO_InitStruct.Pin = MQ135_DIGITAL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MQ135_DIGITAL_PORT, &GPIO_InitStruct);

  // Khởi tạo ADC
  HAL_ADC_Start(&hadc1);

  // Cập nhật trạng thái hệ thống
  g_sensor_system.active_sensor_count = 2;
  g_sensor_system.last_update = HAL_GetTick();

  printf("✓ Khởi tạo MQ137 (NH3): %s\r\n", g_sensor_system.mq137.base.is_enabled ? "OK" : "FAIL");
  printf("✓ Khởi tạo MQ135 (CO2): %s\r\n", g_sensor_system.mq135.base.is_enabled ? "OK" : "FAIL");
  printf("✓ Cảm biến hoạt động: %d/2\r\n", g_sensor_system.active_sensor_count);

  // Thời gian ổn định
  printf("✓ Đang ổn định cảm biến...\r\n");
  for (int i = 30; i > 0; i--)
  {
    printf("Ổn định: %d giây\r", i);
    HAL_Delay(1000);
  }
  printf("\r\n✓ Hệ thống sẵn sàng!\r\n");
}

/**
 * @brief Khởi tạo cảm biến MQ137 (NH3)
 */
void InitMQ137Sensor(GasSensor_t *sensor)
{
  // Cấu hình base sensor
  sensor->base.sensor_id = 0;
  sensor->base.sensor_type = SENSOR_TYPE_MQ137;
  strcpy(sensor->base.sensor_name, "MQ137");
  sensor->base.is_enabled = 1;
  sensor->base.adc_channel = MQ137_ADC_CHANNEL;
  sensor->base.digital_port = MQ137_DIGITAL_PORT;
  sensor->base.digital_pin = MQ137_DIGITAL_PIN;
  sensor->base.r0_value = 10000.0f; // Giá trị mặc định, cần hiệu chuẩn

  // Cấu hình gas sensor
  sensor->gas_type = GAS_TYPE_NH3;
  strcpy(sensor->gas_name, "NH3");
  strcpy(sensor->unit, "ppm");

  // Ngưỡng cảnh báo NH3
  sensor->threshold_low = 15.0f;
  sensor->threshold_high = 25.0f;
  sensor->threshold_danger = 50.0f;

  // Thông số đặc tuyến MQ137 cho NH3
  sensor->curve_a = 102.2f;
  sensor->curve_b = -2.473f;
  sensor->min_ppm = 0.0f;
  sensor->max_ppm = 500.0f;

  // Khởi tạo giá trị
  sensor->alarm_level = ALARM_NORMAL;
}

/**
 * @brief Khởi tạo cảm biến MQ135 (CO2)
 */
void InitMQ135Sensor(GasSensor_t *sensor)
{
  // Cấu hình base sensor
  sensor->base.sensor_id = 1;
  sensor->base.sensor_type = SENSOR_TYPE_MQ135;
  strcpy(sensor->base.sensor_name, "MQ135");
  sensor->base.is_enabled = 1;
  sensor->base.adc_channel = MQ135_ADC_CHANNEL;
  sensor->base.digital_port = MQ135_DIGITAL_PORT;
  sensor->base.digital_pin = MQ135_DIGITAL_PIN;
  sensor->base.r0_value = 10000.0f; // Giá trị mặc định, cần hiệu chuẩn

  // Cấu hình gas sensor
  sensor->gas_type = GAS_TYPE_CO2;
  strcpy(sensor->gas_name, "CO2");
  strcpy(sensor->unit, "ppm");

  // Ngưỡng cảnh báo CO2
  sensor->threshold_low = 800.0f;
  sensor->threshold_high = 1200.0f;
  sensor->threshold_danger = 2000.0f;

  // Thông số đặc tuyến MQ135 cho CO2
  sensor->curve_a = 116.6f;
  sensor->curve_b = -2.769f;
  sensor->min_ppm = 300.0f;
  sensor->max_ppm = 5000.0f;

  // Khởi tạo giá trị
  sensor->alarm_level = ALARM_NORMAL;
}

/**
 * @brief Xử lý tất cả cảm biến
 */
void ProcessAllSensors(void)
{
  uint32_t current_time = HAL_GetTick();

  // Xử lý từng cảm biến
  if (g_sensor_system.mq137.base.is_enabled)
  {
    ProcessGasSensor(&g_sensor_system.mq137);
  }

  if (g_sensor_system.mq135.base.is_enabled)
  {
    ProcessGasSensor(&g_sensor_system.mq135);
  }

  // Cập nhật trạng thái hệ thống
  UpdateSystemStatus();

  // Lưu lịch sử
  SaveToHistory();

  // Cập nhật thống kê
  g_sensor_system.total_readings++;
  g_sensor_system.last_update = current_time;
  g_sensor_system.uptime_minutes = current_time / 60000.0f;
}

/**
 * @brief Xử lý dữ liệu một cảm biến khí
 */
void ProcessGasSensor(GasSensor_t *sensor)
{
  // Đọc dữ liệu thô
  sensor->base.raw_voltage = ReadADC_Voltage(sensor->base.adc_channel);
  sensor->base.digital_state = ReadDigitalState(sensor->base.digital_port, sensor->base.digital_pin);
  sensor->base.timestamp = HAL_GetTick();

  // Tính toán điện trở và tỷ lệ Rs/R0
  if (sensor->base.raw_voltage > 0.2f)
  { // Ngưỡng thấp hơn cho ADC 3.3V
    sensor->base.resistance = CalculateResistance(sensor->base.raw_voltage);
    sensor->base.rs_r0_ratio = sensor->base.resistance / sensor->base.r0_value;

    // Tính nồng độ khí
    sensor->gas_ppm = CalculateGasPPM(sensor->base.rs_r0_ratio, sensor->curve_a, sensor->curve_b);

    // Giới hạn giá trị trong khoảng hợp lệ
    if (sensor->gas_ppm < sensor->min_ppm)
      sensor->gas_ppm = sensor->min_ppm;
    if (sensor->gas_ppm > sensor->max_ppm)
      sensor->gas_ppm = sensor->max_ppm;

    // Lọc nhiễu đơn giản (moving average)
    sensor->filtered_ppm = 0.8f * sensor->filtered_ppm + 0.2f * sensor->gas_ppm;
    sensor->average_ppm = sensor->filtered_ppm;
  }
  else
  {
    sensor->base.resistance = 0.0f;
    sensor->base.rs_r0_ratio = 0.0f;
    sensor->gas_ppm = sensor->min_ppm;
    sensor->filtered_ppm = sensor->min_ppm;
  }

  // Validate dữ liệu
  sensor->base.is_valid = ValidateGasSensor(sensor);

  // Cập nhật mức cảnh báo
  UpdateAlarmLevel(sensor);
}

/**
 * @brief Validate dữ liệu cảm biến (cập nhật cho điện áp 3.3V)
 */
uint8_t ValidateGasSensor(GasSensor_t *sensor)
{
  // Kiểm tra điện áp trong khoảng hợp lệ (0.1V - 4.8V cho cảm biến 5V)
  // Sau khi qua mạch chia áp, tín hiệu sẽ từ 0.066V - 3.168V tại ADC
  if (sensor->base.raw_voltage < 0.2f || sensor->base.raw_voltage > 4.8f)
  {
    return 0;
  }

  // Kiểm tra điện trở trong khoảng hợp lệ
  if (sensor->base.resistance < 1000.0f || sensor->base.resistance > 200000.0f)
  {
    return 0;
  }

  // Kiểm tra nồng độ khí trong khoảng hợp lệ
  if (sensor->gas_ppm < sensor->min_ppm || sensor->gas_ppm > sensor->max_ppm)
  {
    return 0;
  }

  return 1; // Dữ liệu hợp lệ
}

/**
 * @brief Cập nhật mức cảnh báo cho cảm biến
 */
void UpdateAlarmLevel(GasSensor_t *sensor)
{
  sensor->alarm_level = DetermineAlarmLevel(sensor->filtered_ppm,
                                            sensor->threshold_low,
                                            sensor->threshold_high,
                                            sensor->threshold_danger);
}

/**
 * @brief Cập nhật trạng thái tổng thể của hệ thống
 */
void UpdateSystemStatus(void)
{
  uint8_t valid_sensors = 0;
  AlarmLevel_t max_alarm = ALARM_NORMAL;

  // Đếm cảm biến hợp lệ và tìm mức cảnh báo cao nhất
  if (g_sensor_system.mq137.base.is_enabled && g_sensor_system.mq137.base.is_valid)
  {
    valid_sensors++;
    if (g_sensor_system.mq137.alarm_level > max_alarm)
    {
      max_alarm = g_sensor_system.mq137.alarm_level;
    }
  }

  if (g_sensor_system.mq135.base.is_enabled && g_sensor_system.mq135.base.is_valid)
  {
    valid_sensors++;
    if (g_sensor_system.mq135.alarm_level > max_alarm)
    {
      max_alarm = g_sensor_system.mq135.alarm_level;
    }
  }

  // Cập nhật trạng thái hệ thống
  if (valid_sensors == 2)
  {
    g_sensor_system.system_status = 3; // Tối ưu
  }
  else if (valid_sensors == 1)
  {
    g_sensor_system.system_status = 2; // Tốt
  }
  else
  {
    g_sensor_system.system_status = 0; // Lỗi
    g_sensor_system.error_count++;
  }

  g_sensor_system.system_alarm = max_alarm;
}

/**
 * @brief Đọc điện áp từ kênh ADC (đã hiệu chỉnh cho mạch chia áp)
 */
float ReadADC_Voltage(uint32_t channel)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    return 0.0f;
  }

  // Đọc nhiều lần và lấy trung bình để giảm nhiễu
  uint32_t adc_sum = 0;
  for (int i = 0; i < 10; i++)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_sum += HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    HAL_Delay(1); // Delay nhỏ giữa các lần đo
  }

  uint32_t adc_avg = adc_sum / 10;

  // Chuyển đổi giá trị ADC sang điện áp thực tế
  float adc_voltage = (float)adc_avg * ADC_VREF / ADC_RESOLUTION;

  // Hiệu chỉnh cho mạch chia áp để có điện áp thực của cảm biến
  float sensor_voltage = adc_voltage / VOLTAGE_DIVIDER_RATIO;

  return sensor_voltage;
}

/**
 * @brief Đọc trạng thái digital
 */
uint8_t ReadDigitalState(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin);
}

/**
 * @brief Tính điện trở cảm biến (hiệu chỉnh cho nguồn 5V)
 */
float CalculateResistance(float voltage)
{
  if (voltage <= 0.1f)
    return 0.0f;

  // Công thức tính điện trở cảm biến với nguồn cấp 5V
  // Rs = (Vcc - Vs) / Vs * RL
  // Trong đó: Vcc = 5V, Vs = điện áp đo được, RL = điện trở tải
  return ((SENSOR_VCC - voltage) / voltage) * LOAD_RESISTANCE;
}

/**
 * @brief Tính nồng độ khí từ tỷ lệ Rs/R0
 */
float CalculateGasPPM(float rs_r0_ratio, float curve_a, float curve_b)
{
  if (rs_r0_ratio <= 0)
    return 0.0f;
  return curve_a * powf(rs_r0_ratio, curve_b);
}

/**
 * @brief Xác định mức cảnh báo
 */
AlarmLevel_t DetermineAlarmLevel(float ppm, float low, float high, float danger)
{
  if (ppm >= danger)
    return ALARM_DANGER;
  if (ppm >= high)
    return ALARM_HIGH;
  if (ppm >= low)
    return ALARM_LOW;
  return ALARM_NORMAL;
}

/**
 * @brief Lưu dữ liệu vào buffer lịch sử
 */
void SaveToHistory(void)
{
  history_buffer[history_index] = g_sensor_system;
  history_index = (history_index + 1) % HISTORY_SIZE;
}

/**
 * @brief Hiển thị trạng thái hệ thống
 */
void DisplaySystemStatus(void)
{
  printf("\r\n=== TRẠNG THÁI HỆ THỐNG CẢM BIẾN ===\r\n");
  printf("Thời gian: %lu ms | Uptime: %.1f phút\r\n",
         HAL_GetTick(), g_sensor_system.uptime_minutes);

  // Hiển thị MQ137
  PrintSensorDetails(&g_sensor_system.mq137);

  // Hiển thị MQ135
  PrintSensorDetails(&g_sensor_system.mq135);

  // Trạng thái tổng thể
  printf("\r\n--- HỆ THỐNG ---\r\n");
  const char *status_text[] = {"LỖI", "Cơ bản", "Tốt", "Tối ưu"};
  const char *alarm_text[] = {"Bình thường", "Thấp", "Cao", "NGUY HIỂM"};

  printf("Trạng thái: %s (%d/2 cảm biến hoạt động)\r\n",
         status_text[g_sensor_system.system_status], g_sensor_system.active_sensor_count);
  printf("Cảnh báo tổng thể: %s\r\n", alarm_text[g_sensor_system.system_alarm]);
  printf("Tổng số đo: %lu | Lỗi: %lu\r\n",
         g_sensor_system.total_readings, g_sensor_system.error_count);

  printf("=====================================\r\n");
}

/**
 * @brief In chi tiết một cảm biến
 */
void PrintSensorDetails(GasSensor_t *sensor)
{
  printf("\r\n--- %s (%s) ---\r\n", sensor->base.sensor_name, sensor->gas_name);
  printf("ID: %d | Trạng thái: %s\r\n",
         sensor->base.sensor_id,
         sensor->base.is_enabled ? (sensor->base.is_valid ? "Hoạt động" : "Lỗi") : "Tắt");
  printf("Điện áp: %.3f V | Điện trở: %.1f Ω\r\n",
         sensor->base.raw_voltage, sensor->base.resistance);
  printf("Rs/R0: %.3f | R0: %.1f Ω\r\n",
         sensor->base.rs_r0_ratio, sensor->base.r0_value);
  printf("Nồng độ: %.1f %s (Lọc: %.1f %s)\r\n",
         sensor->gas_ppm, sensor->unit, sensor->filtered_ppm, sensor->unit);
  printf("Digital: %s\r\n", sensor->base.digital_state ? "HIGH" : "LOW");

  const char *alarm_text[] = {"Bình thường", "Thấp", "Cao", "NGUY HIỂM"};
  printf("Cảnh báo: %s (%.1f/%.1f/%.1f)\r\n",
         alarm_text[sensor->alarm_level],
         sensor->threshold_low, sensor->threshold_high, sensor->threshold_danger);
}

/**
 * @brief Test hệ thống
 */
void TestSensorSystem(void)
{
  printf("\r\n=== TEST HỆ THỐNG CẢM BIẾN ===\r\n");

  for (int i = 0; i < 10; i++)
  {
    printf("Test lần %d:\r\n", i + 1);

    ProcessAllSensors();

    printf("  MQ137: %.1f ppm NH3 (Alarm: %d)\r\n",
           GetNH3_PPM(), GetNH3AlarmLevel());
    printf("  MQ135: %.1f ppm CO2 (Alarm: %d)\r\n",
           GetCO2_PPM(), GetCO2AlarmLevel());
    printf("  Hệ thống: Status=%d, Alarm=%d\r\n\r\n",
           GetSystemStatus(), GetSystemAlarmLevel());

    HAL_Delay(3000);
  }

  printf("✓ Test hoàn tất!\r\n");
}

/**
 * @brief Hiệu chuẩn cảm biến
 */
void CalibrateSensors(void)
{
  printf("\r\n=== HIỆU CHUẨN CẢM BIẾN ===\r\n");
  printf("Đặt cảm biến trong không khí sạch...\r\n");
  printf("Đang đo R0 trong 30 giây...\r\n");

  float mq137_r0_sum = 0;
  float mq135_r0_sum = 0;
  int valid_samples = 0;

  for (int i = 0; i < 30; i++)
  {
    ProcessAllSensors();

    if (g_sensor_system.mq137.base.is_valid && g_sensor_system.mq135.base.is_valid)
    {
      mq137_r0_sum += g_sensor_system.mq137.base.resistance;
      mq135_r0_sum += g_sensor_system.mq135.base.resistance;
      valid_samples++;
    }

    printf("Mẫu %d: MQ137=%.1fΩ, MQ135=%.1fΩ\r\n",
           i + 1, g_sensor_system.mq137.base.resistance, g_sensor_system.mq135.base.resistance);

    HAL_Delay(1000);
  }

  if (valid_samples > 0)
  {
    float mq137_r0_avg = mq137_r0_sum / valid_samples;
    float mq135_r0_avg = mq135_r0_sum / valid_samples;

    // Cập nhật giá trị R0
    g_sensor_system.mq137.base.r0_value = mq137_r0_avg;
    g_sensor_system.mq135.base.r0_value = mq135_r0_avg;

    printf("\r\n=== KẾT QUẢ HIỆU CHUẨN ===\r\n");
    printf("MQ137 R0: %.1f Ω\r\n", mq137_r0_avg);
    printf("MQ135 R0: %.1f Ω\r\n", mq135_r0_avg);
    printf("Mẫu hợp lệ: %d/30\r\n", valid_samples);
    printf("\r\nCập nhật trong code:\r\n");
    printf("#define MQ137_R0 %.1ff\r\n", mq137_r0_avg);
    printf("#define MQ135_R0 %.1ff\r\n", mq135_r0_avg);
    printf("✓ Hiệu chuẩn thành công!\r\n");
  }
  else
  {
    printf("❌ Hiệu chuẩn thất bại - không có mẫu hợp lệ!\r\n");
  }
}

// =============================================================================
// GETTER FUNCTIONS
// =============================================================================

/**
 * @brief Lấy nồng độ NH3 hiện tại
 */
float GetNH3_PPM(void)
{
  return g_sensor_system.mq137.base.is_valid ? g_sensor_system.mq137.filtered_ppm : 0.0f;
}

/**
 * @brief Lấy nồng độ CO2 hiện tại
 */
float GetCO2_PPM(void)
{
  return g_sensor_system.mq135.base.is_valid ? g_sensor_system.mq135.filtered_ppm : 0.0f;
}

/**
 * @brief Lấy mức cảnh báo NH3
 */
AlarmLevel_t GetNH3AlarmLevel(void)
{
  return g_sensor_system.mq137.alarm_level;
}

/**
 * @brief Lấy mức cảnh báo CO2
 */
AlarmLevel_t GetCO2AlarmLevel(void)
{
  return g_sensor_system.mq135.alarm_level;
}

/**
 * @brief Lấy mức cảnh báo tổng thể
 */
AlarmLevel_t GetSystemAlarmLevel(void)
{
  return g_sensor_system.system_alarm;
}

/**
 * @brief Lấy trạng thái hệ thống
 */
uint8_t GetSystemStatus(void)
{
  return g_sensor_system.system_status;
}

// =============================================================================
// HÀM MAIN
// =============================================================================

/**
 * @brief Hàm main chính
 */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();

  printf("\r\n╔════════════════════════════════════════╗\r\n");
  printf("║      HỆ THỐNG GIÁM SÁT KHÍ NH3 & CO2   ║\r\n");
  printf("║      Phiên bản: 3.0 - Modular         ║\r\n");
  printf("║      Cảm biến: MQ137 + MQ135          ║\r\n");
  printf("║      ADC: 3.3V (cần mạch chia áp)     ║\r\n");
  printf("╚════════════════════════════════════════╝\r\n");

  printf("\r\n⚠️  LƯU Ý QUAN TRỌNG:\r\n");
  printf("   - STM32F103 chỉ đo ADC 0-3.3V\r\n");
  printf("   - Cảm biến MQ hoạt động ở 5V\r\n");
  printf("   - CẦN mạch chia áp cho AOUT:\r\n");
  printf("     AOUT → R1(2.7kΩ) → ADC_PIN → R2(5.1kΩ) → GND\r\n");
  printf("   - Hoặc dùng Op-Amp buffer với gain = 0.66\r\n\r\n");

  // Khởi tạo hệ thống
  InitSensorSystem();

  // Test hệ thống
  TestSensorSystem();

  // Hiệu chuẩn nếu cần (uncomment để chạy)
  // CalibrateSensors();

  printf("\r\n🚀 BẮT ĐẦU GIÁM SÁT LIÊN TỤC\r\n");
  printf("Press any key to stop...\r\n");

  while (1)
  {
    // Xử lý tất cả cảm biến
    ProcessAllSensors();

    // Hiển thị trạng thái
    DisplaySystemStatus();

    // Lấy giá trị để điều khiển
    float nh3_ppm = GetNH3_PPM();
    float co2_ppm = GetCO2_PPM();
    AlarmLevel_t nh3_alarm = GetNH3AlarmLevel();
    AlarmLevel_t co2_alarm = GetCO2AlarmLevel();
    AlarmLevel_t system_alarm = GetSystemAlarmLevel();

    // Logic điều khiển NH3
    if (nh3_alarm >= ALARM_HIGH)
    {
      printf("🚨 NH3 = %.1f ppm - BẬT QUẠT THÔNG GIÓ!\r\n", nh3_ppm);
      // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_SET);
    }
    else if (nh3_alarm <= ALARM_NORMAL)
    {
      printf("✅ NH3 = %.1f ppm - Bình thường\r\n", nh3_ppm);
      // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_RESET);
    }

    // Logic điều khiển CO2
    if (co2_alarm >= ALARM_HIGH)
    {
      printf("🚨 CO2 = %.1f ppm - CẦN THÔNG GIÓ!\r\n", co2_ppm);
      // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_SET);
    }
    else if (co2_alarm <= ALARM_NORMAL)
    {
      printf("✅ CO2 = %.1f ppm - Bình thường\r\n", co2_ppm);
      // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_RESET);
    }

    // Cảnh báo tổng thể
    switch (system_alarm)
    {
    case ALARM_DANGER:
      printf("🚨🚨 NGUY HIỂM! KIỂM TRA NGAY LẬP TỨC! 🚨🚨\r\n");
      // HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
      break;
    case ALARM_HIGH:
      printf("⚠️ CẢNH BÁO CAO - Cần chú ý ngay\r\n");
      break;
    case ALARM_LOW:
      printf("⚠️ Cảnh báo thấp - Theo dõi\r\n");
      break;
    default:
      printf("✅ Hệ thống hoạt động bình thường\r\n");
      // HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
      break;
    }

    // TODO: Mở rộng tính năng
    // - Lưu dữ liệu vào SD card
    // SaveDataToSD(&g_sensor_system);

    // - Gửi dữ liệu qua WiFi/Bluetooth
    // SendDataToServer(nh3_ppm, co2_ppm, system_alarm);

    // - Hiển thị trên LCD
    // UpdateLCDDisplay(nh3_ppm, co2_ppm, system_alarm);

    // - Log dữ liệu
    // LogToFile(&g_sensor_system);

    printf("\r\n⏳ Chờ 60 giây để đo tiếp...\r\n");
    printf("────────────────────────────────────────\r\n");

    HAL_Delay(60000); // Chờ 1 phút
  }
}