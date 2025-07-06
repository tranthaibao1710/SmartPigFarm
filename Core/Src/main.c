/* =============================================================================
 * MAIN.C - MAIN IMPLEMENTATION FILE
 * Hệ thống giám sát khí MQ137 & MQ135 - Modular Design
 * Phiên bản: 3.0
 * =============================================================================
 */

#include "sensor_system.h"

// =============================================================================
// PRIVATE INCLUDES
// =============================================================================

// =============================================================================
// PRIVATE TYPEDEF
// =============================================================================

// =============================================================================
// PRIVATE DEFINE
// =============================================================================

// =============================================================================
// PRIVATE MACRO
// =============================================================================

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

// =============================================================================
// PRIVATE FUNCTION PROTOTYPES
// =============================================================================

// =============================================================================
// PRIVATE USER CODE
// =============================================================================

/**
 * @brief The application entry point.
 * @retval int
 */
int main(void)
{
  // HAL Initialization
  HAL_Init();
  SystemClock_Config();
  USARTx_Init(USART1, PA9PA10, 115200);

  // Display welcome message
  printf("HE THONG GIAM SAT KHI NH3 & CO2\n");
  printf("CAM BIEN: MQ137 + MQ135          \r\n");
 

  // Khởi tạo hệ thống cảm biến
  InitSensorSystem();

  // Test hệ thống
  TestSensorSystem();

  // Hiệu chuẩn nếu cần (uncomment để chạy)
  CalibrateSensors();

  printf("\r\n BAT DAU GIAM SAT\r\n");
  printf("Press any key to stop...\r\n");

  // Main loop
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
 // Debug thong tin
    printf("DEBUG - NH3: %.1f ppm, Alarm Level: %d\r\n", nh3_ppm, nh3_alarm);
    printf("DEBUG - CO2: %.1f ppm, Alarm Level: %d\r\n", co2_ppm, co2_alarm);
    
    // Logic dieu khien NH3 - day du tat ca truong hop
    printf("\r\nNH3 STATUS: ");
    if(nh3_alarm == ALARM_DANGER) {
        printf("NH3 = %.1f ppm - NGUY HIEM! BAT QUAT MAX!\r\n", nh3_ppm);
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_SET);
    }
    else if(nh3_alarm == ALARM_HIGH) {
        printf("NH3 = %.1f ppm - BAT QUAT THONG GIO!\r\n", nh3_ppm);
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_SET);
    }
    else if(nh3_alarm == ALARM_LOW) {
        printf("NH3 = %.1f ppm - Canh bao thap\r\n", nh3_ppm);
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_RESET);
    }
    else { // ALARM_NORMAL
        printf("NH3 = %.1f ppm - Binh thuong\r\n", nh3_ppm);
        // HAL_GPIO_WritePin(FAN_NH3_PORT, FAN_NH3_PIN, GPIO_PIN_RESET);
    }
    
    // Logic dieu khien CO2 - day du tat ca truong hop
    printf("CO2 STATUS: ");
    if(co2_alarm == ALARM_DANGER) {
        printf("CO2 = %.1f ppm - NGUY HIEM! CAN THONG GIO NGAY!\r\n", co2_ppm);
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_SET);
    }
    else if(co2_alarm == ALARM_HIGH) {
        printf("CO2 = %.1f ppm - BAT QUAT THONG GIO!\r\n", co2_ppm);
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_SET);
    }
    else if(co2_alarm == ALARM_LOW) {
        printf("CO2 = %.1f ppm - Canh bao thap\r\n", co2_ppm);
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_RESET);
    }
    else { // ALARM_NORMAL
        printf("CO2 = %.1f ppm - Binh thuong\r\n", co2_ppm);
        // HAL_GPIO_WritePin(FAN_CO2_PORT, FAN_CO2_PIN, GPIO_PIN_RESET);
    }

    // Cảnh báo tổng thể
    switch (system_alarm)
    {
    case ALARM_DANGER:
      printf("NGUY HIEM - KIEM TRA NGAY LAP TUC\r\n");
      // HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
      break;
    case ALARM_HIGH:
      printf("CANH BAO CAO - CHU Y NGAY\r\n");
      break;
    case ALARM_LOW:
      printf("CANH BAO THAP - TIEP TUC THEO DOI\r\n");
      break;
    default:
      printf("HE THONG HOAT DONG BINH THUONG\r\n");
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

    printf("\r\nCHO 60 GIAY DE DO TIEP\r\n");
    

    HAL_Delay(MAIN_LOOP_DELAY); // Chờ 1 phút
  }
}

// =============================================================================
// SYSTEM CONFIGURATION FUNCTIONS
// =============================================================================

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
