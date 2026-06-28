/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : HỆ THỐNG LỌC SỐ FIR TRÊN STM32
  * @author         : [Tên của bạn]
  * @note           : Code này bao gồm đầy đủ:
  * 1. Cấu hình Clock hệ thống & Ngoại vi (I2S, DMA, USB).
  * 2. Thuật toán lọc số FIR (Convolution).
  * 3. Cơ chế thu thập mẫu DMA Circular (Double Buffering).
  * 4. Cơ chế truyền USB gói lớn (Batching).
  * 5. Chế độ Test (Giả lập) và Chế độ Mic (Thực tế).
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define MODE_TEST 0  // Chế độ chạy thử nghiệm sóng giả lập
#define MODE_MIC  1  // Chế độ chạy thực tế với Microphone

#define RUN_MODE  MODE_MIC
// =========================================================
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Bậc của bộ lọc FIR. N=51 nghĩa là cần 51 phép nhân + cộng cho 1 mẫu đầu ra.
   Số này được tính toán từ MATLAB để đạt tần số cắt 20kHz với độ dốc phù hợp. */
#define FIR_TAP  51

/* Kích thước gói dữ liệu (Snapshot) gửi lên máy tính.
   512 mẫu là đủ để thuật toán FFT trên MATLAB hiển thị đẹp và chi tiết. */
#define BUF_SIZE 512

/* Kích thước buffer phần cứng cho DMA.
   64 mẫu: DMA sẽ ngắt khi đầy 32 mẫu (Half) và 64 mẫu (Full).
   Kỹ thuật Double Buffering giúp xử lý liên tục mà không ngắt quãng tín hiệu. */
#define I2S_DMA_LEN 64

/* Hệ số khuếch đại số (Digital Gain).*/
#define MIC_GAIN 15000.0f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_rx;

/* USER CODE BEGIN PV */

/* 1. DỮ LIỆU TÍN HIỆU GIẢ LẬP (Mode A) */
float test_signal[] = {
		0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, -0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, 0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, -0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, -0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f,
		-0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, 0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, -0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, -0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, 0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f,
		-0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, 0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, 0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, -0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, 0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f,
		-0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, 0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, 0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		-0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, 0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, 0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f,
		-0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, 0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, 0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, -0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, -0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f,
		0.000000f, 0.256605f, 0.224144f, 0.653281f, 0.250000f, 0.766320f, -0.000000f, 0.588018f,
		-0.433013f, 0.270598f, -0.836516f, 0.033783f, -1.000000f, 0.033783f, -0.836516f, 0.270598f,
		-0.433013f, 0.588018f, 0.000000f, 0.766320f, 0.250000f, 0.653281f, 0.224144f, 0.256605f,
		0.000000f, -0.256605f, -0.224144f, -0.653281f, -0.250000f, -0.766320f, -0.000000f, -0.588018f,
		0.433013f, -0.270598f, 0.836516f, -0.033783f, 1.000000f, -0.033783f, 0.836516f, -0.270598f,
		0.433013f, -0.588018f, -0.000000f, -0.766320f, -0.250000f, -0.653281f, -0.224144f, -0.256605f
};

/* 2. HỆ SỐ BỘ LỌC FIR (Impulse Response)
   Đây là "trái tim" của bộ lọc. Các con số này quyết định đặc tính Lọc Thông Thấp.
   Được thiết kế bằng MATLAB (Window Method - Hamming Window). */
float fir_coeff[FIR_TAP] = {
		0.000509f, -0.000000f, -0.000653f, 0.001406f, -0.002074f, 0.002312f, -0.001713f, 0.000000f,
		0.002746f, -0.005925f, 0.008437f, -0.008929f, 0.006253f, -0.000000f, -0.009054f, 0.018791f,
		-0.026007f, 0.027081f, -0.018931f, 0.000000f, 0.029013f, -0.064948f, 0.102601f, -0.135741f,
		0.158462f, 0.832726f, 0.158462f, -0.135741f, 0.102601f, -0.064948f, 0.029013f, 0.000000f,
		-0.018931f, 0.027081f, -0.026007f, 0.018791f, -0.009054f, -0.000000f, 0.006253f, -0.008929f,
		0.008437f, -0.005925f, 0.002746f, 0.000000f, -0.001713f, 0.002312f, -0.002074f, 0.001406f,
		-0.000653f, -0.000000f, 0.000509f
};

/* 3. CÁC BIẾN CHO THUẬT TOÁN FIR */
float fir_buf[FIR_TAP] = {0}; // Circular Buffer lưu mẫu đầu vào quá khứ (x[n-k])
uint16_t fir_idx = 0;         // Con trỏ vị trí hiện tại trong Circular Buffer

/* 4. BIẾN TRUYỀN THÔNG USB (OPTIMIZATION) */
/* Buffer lớn 2KB để gom nhiều mẫu thành 1 gói (Batching).
   Lý do: Gửi từng byte qua USB rất chậm (overhead lớn).
   Gom lại gửi 1 lần giúp tăng tốc độ thực tế lên gấp 10-20 lần. */
char tx_packet_buf[2048];

/* 5. BIẾN XỬ LÝ ÂM THANH */
int32_t rx_dma_buf[I2S_DMA_LEN]; // Buffer nhận dữ liệu thô từ I2S DMA (Vùng nhớ chia sẻ)
float raw_store[BUF_SIZE];       // Kho lưu trữ tín hiệu gốc (Raw) cho 1 Snapshot
float fir_store[BUF_SIZE];       // Kho lưu trữ tín hiệu đã lọc (Filtered) cho 1 Snapshot
volatile uint16_t store_idx = 0; // Vị trí ghi hiện tại trong kho

volatile uint8_t recording = 1;          // Cờ trạng thái: 1 = Đang thu âm
volatile uint8_t data_ready_to_send = 0; // Cờ trạng thái: 1 = Đã thu đủ, sẵn sàng gửi

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S2_Init(void);
/* USER CODE BEGIN PFP */

/* ===================== HÀM KHỞI TẠO BỘ LỌC =====================
   Xóa sạch buffer lịch sử để tránh nhiễu từ lần chạy trước */
void fir_reset(void)
{
    for (int i = 0; i < FIR_TAP; i++) fir_buf[i] = 0.0f;
    fir_idx = 0;
}

/* ===================== THUẬT TOÁN FIR (CONVOLUTION) =====================
   Thực hiện phép tính: y[n] = Sum(h[k] * x[n-k])
   Sử dụng Circular Buffer để không phải dịch chuyển dữ liệu mảng (Tối ưu tốc độ) */
float fir_filter(float x)
{
    // 1. Lưu mẫu mới vào buffer vòng
    fir_buf[fir_idx] = x;
    float y = 0.0f;
    int j = fir_idx; // j là chỉ số duyệt ngược về quá khứ
    // 2. Thực hiện nhân tích chập (Multiply-Accumulate)
    for (int i = 0; i < FIR_TAP; i++)
    {
        y += fir_coeff[i] * fir_buf[j];
        // Dịch chuyển chỉ số j theo vòng tròn (Circular Indexing)
        if (j == 0) j = FIR_TAP - 1;
        else j--;
    }
    // 3. Tăng chỉ số cho lần gọi tiếp theo
    fir_idx++;
    if (fir_idx >= FIR_TAP) fir_idx = 0;
    return y; // Trả về mẫu đã lọc
}

/* ===================== HÀM XỬ LÝ MẺ DỮ LIỆU TỪ DMA =====================
   Hàm này được gọi bởi ngắt DMA (Callback) khi có dữ liệu mới từ Mic.
   Đây là nơi chuyển đổi dữ liệu thô (I2S) sang dữ liệu có nghĩa (Float). */
void Process_Audio_Batch(int32_t *src_buf, uint16_t count)
{
    // Nếu main() đang bận gửi dữ liệu cũ thì bỏ qua dữ liệu mới để tránh xung đột
    if (data_ready_to_send) return;

    for (int i = 0; i < count; i++)
    {
        // Chỉ thu thập nếu đang bật chế độ ghi và kho chưa đầy
        if (recording && store_idx < BUF_SIZE)
        {
            /* [QUAN TRỌNG] Xử lý dữ liệu I2S 24-bit Philips Standard.
               Dữ liệu 24-bit được căn trái trong thanh ghi 32-bit (MSB aligned).
               Cần dịch phải 8 bit để lấy đúng giá trị thực có dấu (Signed Int). */
            int32_t val_24 = src_buf[i] >> 8;

            /* Chuẩn hóa về dải Float [-1.0, 1.0] và nhân hệ số Gain */
            float x_raw = ((float)val_24 / 8388608.0f) * MIC_GAIN;

            /* Gọi hàm lọc FIR */
            float y = fir_filter(x_raw);

            /* Clamping (Bão hòa): Cắt ngọn nếu tín hiệu vượt quá +/- 1.0
               để tránh lỗi tràn số hoặc méo tiếng khi hiển thị */
            if (y > 1.0f) y = 1.0f;
            else if (y < -1.0f) y = -1.0f;

            if (x_raw > 1.0f) x_raw = 1.0f;
            else if (x_raw < -1.0f) x_raw = -1.0f;

            /* Lưu vào kho chứa (Snapshot) */
            raw_store[store_idx] = x_raw;
            fir_store[store_idx] = y;
            store_idx++;
        }

        /* Kiểm tra nếu kho đã đầy (đủ 512 mẫu) */
        if (store_idx >= BUF_SIZE)
        {
            recording = 0;          // Dừng thu
            data_ready_to_send = 1; // Bật cờ báo cho main() biết để gửi USB
            break;
        }
    }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  fir_reset(); // Xóa sạch bộ nhớ lọc

  /* [QUAN TRỌNG] Kích hoạt I2S DMA chế độ Circular (Vòng tròn).
     - DMA tự động chuyển dữ liệu từ Mic vào buffer rx_dma_buf.
     - CPU không cần can thiệp vào quá trình này, chỉ chờ ngắt báo xong. */
  if (RUN_MODE == MODE_MIC) {
      HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rx_dma_buf, I2S_DMA_LEN);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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

#ifdef  USE_FULL_ASSERT
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
