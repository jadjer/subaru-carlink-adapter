#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

constexpr auto NON_STB_PIN = GPIO_NUM_9;
constexpr auto BUS_RX_PIN = GPIO_NUM_8;
constexpr auto BUS_TX_PIN = GPIO_NUM_3;
constexpr auto OUTPUT_ENABLE_PIN = GPIO_NUM_6;

static const char* TAG = "PulseMonitor";

// Глобальные переменные для хранения времени
static uint64_t last_high_time = 0;
static uint64_t last_high_duration = 0;
static uint64_t last_period = 0;

class DataReceiver {
private:
    static constexpr gpio_num_t DATA_PIN = GPIO_NUM_8;
    static constexpr uint32_t START_PULSE_MIN = 168000;  // 168 мкс
    static constexpr uint32_t START_PULSE_MAX = 172000;  // 172 мкс
    static constexpr uint32_t BIT1_PULSE_MIN = 18000;    // 18 мкс
    static constexpr uint32_t BIT1_PULSE_MAX = 22000;    // 22 мкс
    static constexpr uint32_t BIT0_PULSE_MIN = 33500;    // 33.5 мкс
    static constexpr uint32_t BIT0_PULSE_MAX = 37000;    // 37 мкс

    enum class ReceiverState {
        IDLE,
        WAITING_START,
        RECEIVING_DATA
    };

    struct Message {
        bool start_bit;
        bool broadcast;
        uint16_t master_address;
        bool master_parity;
        uint16_t receiver_address;
        bool receiver_parity;
        bool ack1;
        uint8_t control_bits;
        bool control_parity;
        bool ack2;
        uint8_t data_length;
        bool length_parity;
        bool ack3;
        uint8_t data[32];
        bool data_parity[32];
        bool data_ack[32];
    };

    ReceiverState state;
    Message current_message;
    uint8_t current_bit_position;
    uint8_t current_data_byte;
    uint32_t pulse_start_time;
    QueueHandle_t data_queue;
    TaskHandle_t processing_task;
    bool is_addressed;

public:
    DataReceiver() : state(ReceiverState::IDLE), current_bit_position(0),
                    current_data_byte(0), is_addressed(false) {
        data_queue = xQueueCreate(10, sizeof(Message));
        xTaskCreate(processingTask, "DataProcessor", 4096, this, 5, &processing_task);
        setupGPIO();
    }

    ~DataReceiver() {
        if (processing_task) {
            vTaskDelete(processing_task);
        }
        if (data_queue) {
            vQueueDelete(data_queue);
        }
    }

private:
    void setupGPIO() {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_ANYEDGE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << DATA_PIN);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);

        gpio_install_isr_service(0);
        gpio_isr_handler_add(DATA_PIN, isrHandler, this);
    }

    static void IRAM_ATTR isrHandler(void* arg) {
        DataReceiver* receiver = static_cast<DataReceiver*>(arg);
        receiver->handleInterrupt();
    }

    void IRAM_ATTR handleInterrupt() {
        uint32_t current_time = esp_timer_get_time();
        int level = gpio_get_level(DATA_PIN);

        if (level == 1) {  // Rising edge - start of pulse
            pulse_start_time = current_time;

            if (state == ReceiverState::IDLE) {
                state = ReceiverState::WAITING_START;
            }
        } else {  // Falling edge - end of pulse
            if (state == ReceiverState::WAITING_START ||
                state == ReceiverState::RECEIVING_DATA) {

                uint32_t pulse_duration = current_time - pulse_start_time;
                processPulse(pulse_duration);
            }
        }
    }

    void IRAM_ATTR processPulse(uint32_t duration) {
        switch (state) {
            case ReceiverState::WAITING_START:
                if (duration >= START_PULSE_MIN && duration <= START_PULSE_MAX) {
                    // Start pulse detected
                    resetMessage();
                    state = ReceiverState::RECEIVING_DATA;
                    ESP_LOGI("Receiver", "Start pulse detected");
                }
                break;

            case ReceiverState::RECEIVING_DATA:
                processDataBit(duration);
                break;

            default:
                break;
        }
    }

    void IRAM_ATTR processDataBit(uint32_t duration) {
        bool bit_value;

        if (duration >= BIT1_PULSE_MIN && duration <= BIT1_PULSE_MAX) {
            bit_value = true;
        } else if (duration >= BIT0_PULSE_MIN && duration <= BIT0_PULSE_MAX) {
            bit_value = false;
        } else {
            // Invalid pulse duration - reset
            ESP_LOGE("Receiver", "Invalid pulse duration: %lu us", duration);
            state = ReceiverState::IDLE;
            return;
        }

        // Process bit according to protocol structure
        switch (current_bit_position) {
            case 0:  // Start bit
                current_message.start_bit = bit_value;
                break;

            case 1:  // Broadcast bit
                current_message.broadcast = bit_value;
                is_addressed = !bit_value;  // Addressed if not broadcast
                break;

            case 2: case 3: case 4: case 5: case 6: case 7:
            case 8: case 9: case 10: case 11: case 12: case 13:  // 12-bit master address
                if (current_bit_position >= 2 && current_bit_position <= 13) {
                    uint8_t bit_pos = current_bit_position - 2;
                    if (bit_value) {
                        current_message.master_address |= (1 << (11 - bit_pos));
                    }
                }
                break;

            case 14:  // Master address parity
                current_message.master_parity = bit_value;
                if (!checkParity(current_message.master_address, 12, bit_value)) {
                    ESP_LOGE("Receiver", "Master address parity error");
                    state = ReceiverState::IDLE;
                    return;
                }
                break;

            case 15: case 16: case 17: case 18: case 19: case 20:
            case 21: case 22: case 23: case 24: case 25: case 26:  // 12-bit receiver address
                if (current_bit_position >= 15 && current_bit_position <= 26) {
                    uint8_t bit_pos = current_bit_position - 15;
                    if (bit_value) {
                        current_message.receiver_address |= (1 << (11 - bit_pos));
                    }
                }
                break;

            case 27:  // Receiver address parity
                current_message.receiver_parity = bit_value;
                if (!checkParity(current_message.receiver_address, 12, bit_value)) {
                    ESP_LOGE("Receiver", "Receiver address parity error");
                    state = ReceiverState::IDLE;
                    return;
                }
                break;

            case 28:  // First acknowledgment
                current_message.ack1 = bit_value;
                if (is_addressed) {
                    sendAckBit();
                }
                break;

            case 29: case 30: case 31: case 32:  // 4 control bits
                if (current_bit_position >= 29 && current_bit_position <= 32) {
                    uint8_t bit_pos = current_bit_position - 29;
                    if (bit_value) {
                        current_message.control_bits |= (1 << (3 - bit_pos));
                    }
                }
                break;

            case 33:  // Control bits parity
                current_message.control_parity = bit_value;
                if (!checkParity(current_message.control_bits, 4, bit_value)) {
                    ESP_LOGE("Receiver", "Control bits parity error");
                    state = ReceiverState::IDLE;
                    return;
                }
                break;

            case 34:  // Second acknowledgment
                current_message.ack2 = bit_value;
                if (is_addressed) {
                    sendAckBit();
                }
                break;

            case 35: case 36: case 37: case 38: case 39: case 40:
            case 41: case 42:  // 8-bit data length
                if (current_bit_position >= 35 && current_bit_position <= 42) {
                    uint8_t bit_pos = current_bit_position - 35;
                    if (bit_value) {
                        current_message.data_length |= (1 << (7 - bit_pos));
                    }
                }
                break;

            case 43:  // Length parity
                current_message.length_parity = bit_value;
                if (!checkParity(current_message.data_length, 8, bit_value)) {
                    ESP_LOGE("Receiver", "Data length parity error");
                    state = ReceiverState::IDLE;
                    return;
                }
                break;

            case 44:  // Third acknowledgment
                current_message.ack3 = bit_value;
                if (is_addressed) {
                    sendAckBit();
                }
                break;

            default:
                // Data bytes processing
                processDataByte(bit_value);
                break;
        }

        current_bit_position++;

        // Check if message is complete
        if (current_bit_position >= (45 + (current_message.data_length * 10))) {
            completeMessage();
        }
    }

    void IRAM_ATTR processDataByte(bool bit_value) {
        uint32_t data_bit_pos = current_bit_position - 45;
        uint8_t byte_index = data_bit_pos / 10;
        uint8_t bit_in_byte = data_bit_pos % 10;

        if (byte_index >= 32) {
            ESP_LOGE("Receiver", "Data overflow");
            state = ReceiverState::IDLE;
            return;
        }

        if (bit_in_byte < 8) {
            // Data bits
            if (bit_value) {
                current_message.data[byte_index] |= (1 << (7 - bit_in_byte));
            }
        } else if (bit_in_byte == 8) {
            // Parity bit
            current_message.data_parity[byte_index] = bit_value;
            if (!checkParity(current_message.data[byte_index], 8, bit_value)) {
                ESP_LOGE("Receiver", "Data byte %d parity error", byte_index);
                state = ReceiverState::IDLE;
                return;
            }
        } else if (bit_in_byte == 9) {
            // Acknowledgment bit
            current_message.data_ack[byte_index] = bit_value;
            if (is_addressed) {
                sendAckBit();
            }
        }
    }

    void IRAM_ATTR completeMessage() {
        // Send message to processing queue
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(data_queue, &current_message, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }

        state = ReceiverState::IDLE;
        ESP_LOGI("Receiver", "Message received, length: %d", current_message.data_length);
    }

    void IRAM_ATTR sendAckBit() {
        // Implementation for sending acknowledgment bit back to network
        // This would depend on your specific hardware setup
        // gpio_set_level(ACK_PIN, 1);
        // ets_delay_us(10);  // Short pulse for ack
        // gpio_set_level(ACK_PIN, 0);
    }

    bool IRAM_ATTR checkParity(uint32_t data, uint8_t num_bits, bool expected_parity) {
        bool parity = false;
        for (uint8_t i = 0; i < num_bits; i++) {
            if (data & (1 << i)) {
                parity = !parity;
            }
        }
        return parity == expected_parity;
    }

    void resetMessage() {
        memset(&current_message, 0, sizeof(Message));
        current_bit_position = 0;
        current_data_byte = 0;
        is_addressed = false;
    }

    static void processingTask(void* arg) {
        DataReceiver* receiver = static_cast<DataReceiver*>(arg);
        receiver->processMessages();
    }

    void processMessages() {
        Message received_message;

        while (true) {
            if (xQueueReceive(data_queue, &received_message, portMAX_DELAY)) {
                // Process the complete message
                ESP_LOGI("Processor", "Processing message:");
                ESP_LOGI("Processor", "  Master: 0x%03X", received_message.master_address);
                ESP_LOGI("Processor", "  Receiver: 0x%03X", received_message.receiver_address);
                ESP_LOGI("Processor", "  Control: 0x%X", received_message.control_bits);
                ESP_LOGI("Processor", "  Length: %d", received_message.data_length);

                if (received_message.data_length > 0) {
                    ESP_LOGI("Processor", "  Data: ");
                    for (int i = 0; i < received_message.data_length; i++) {
                        ESP_LOGI("Processor", "    [%d]: 0x%02X", i, received_message.data[i]);
                    }
                }

                // Here you can add your application-specific message processing
            }
        }
    }
};

extern "C" [[noreturn]] void app_main() {
    // Настройка пина
    gpio_config_t io_conf = {};

    {
        io_conf.pin_bit_mask = (1ULL << OUTPUT_ENABLE_PIN);
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io_conf);
        gpio_set_level(OUTPUT_ENABLE_PIN, 1);
    }

    {
        io_conf.pin_bit_mask = (1ULL << NON_STB_PIN);
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io_conf);
        gpio_set_level(NON_STB_PIN, 1);
    }

    {
        io_conf.pin_bit_mask = (1ULL << BUS_TX_PIN);
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io_conf);
        gpio_set_level(BUS_TX_PIN, 0);
    }

    {
        io_conf.pin_bit_mask = (1ULL << BUS_RX_PIN);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_ANYEDGE;
        gpio_config(&io_conf);
    }

    DataReceiver receiver;

    // Keep the task alive
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
