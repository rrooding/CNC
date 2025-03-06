#include <vector>
#include <string>
#include <string.h>
#include "GrblParserC.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <math.h>

// GPIO pin definitions
#define ENCODER_A_NODE DT_PATH(gpio_pendant, encoder_a)
#define ENCODER_B_NODE DT_PATH(gpio_pendant, encoder_b)
#define ENCODER_A_INV_NODE DT_PATH(gpio_pendant, encoder_a_inv)
#define ENCODER_B_INV_NODE DT_PATH(gpio_pendant, encoder_b_inv)

#define AXIS_X_NODE DT_PATH(gpio_pendant, axis_x)
#define AXIS_Y_NODE DT_PATH(gpio_pendant, axis_y)
#define AXIS_Z_NODE DT_PATH(gpio_pendant, axis_z)

#define SCALE_X1_NODE DT_PATH(gpio_pendant, scale_x1)
#define SCALE_X10_NODE DT_PATH(gpio_pendant, scale_x10)
#define SCALE_X100_NODE DT_PATH(gpio_pendant, scale_x100)

// #define ESTOP_PIN        25   // ESP32 GPIO25 for E-stop (Blue-Black)

#define UART_DEVICE DT_NODELABEL(uart1)

static volatile int encoder_pos = 0;
static char selected_axis = 'O';  // Default to X
static int scale_factor = 1;      // Default to x1

static const struct gpio_dt_spec encoder_a = GPIO_DT_SPEC_GET(ENCODER_A_NODE, gpios);
static const struct gpio_dt_spec encoder_b = GPIO_DT_SPEC_GET(ENCODER_B_NODE, gpios);
static const struct gpio_dt_spec encoder_a_inv = GPIO_DT_SPEC_GET(ENCODER_A_INV_NODE, gpios);
static const struct gpio_dt_spec encoder_b_inv = GPIO_DT_SPEC_GET(ENCODER_B_INV_NODE, gpios);

static const struct gpio_dt_spec axis_x = GPIO_DT_SPEC_GET(AXIS_X_NODE, gpios);
static const struct gpio_dt_spec axis_y = GPIO_DT_SPEC_GET(AXIS_Y_NODE, gpios);
static const struct gpio_dt_spec axis_z = GPIO_DT_SPEC_GET(AXIS_Z_NODE, gpios);

static const struct gpio_dt_spec scale_x1 = GPIO_DT_SPEC_GET(SCALE_X1_NODE, gpios);
static const struct gpio_dt_spec scale_x10 = GPIO_DT_SPEC_GET(SCALE_X10_NODE, gpios);
static const struct gpio_dt_spec scale_x100 = GPIO_DT_SPEC_GET(SCALE_X100_NODE, gpios);

// /* Encoder interrupt callback */
static void encoder_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    bool a = gpio_pin_get_dt(&encoder_a);
    bool b = gpio_pin_get_dt(&encoder_b);

    if (a != b) {
        encoder_pos++;  // Clockwise
    } else {
        encoder_pos--;  // Counterclockwise
    }
	k_msleep(1);
}

// Pin control configuration
PINCTRL_DT_DEFINE(UART_DEVICE);

// UART pin configuration structure
static const struct pinctrl_dev_config *pinctrl_cfg = PINCTRL_DT_DEV_CONFIG_GET(UART_DEVICE);

class FluidNCController {
private:
	const struct device *uart_dev;

	struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
	};

	bool init_uart() {
		uart_dev = DEVICE_DT_GET(UART_DEVICE);
		if (!device_is_ready(uart_dev)) {
			printk("UART device not ready!\n");
			return false;
		}

        int ret = pinctrl_apply_state(pinctrl_cfg, PINCTRL_STATE_DEFAULT);
        if (ret < 0) {
            printk("Failed to configure UART pins: %d\n", ret);
            return false;
        }

		ret = uart_configure(uart_dev, &uart_cfg);
		if (ret < 0) {
			printk("Failed to configure UART: %d\n", ret);
			return false;
		}

		return true;
	}

    /* Generate G-Code and send to console */
    void generate_gcode(int steps) {
        char gcode[64];
        float step_size = 0.001;  // Base step size in mm (adjust based on your encoder resolution)
        float movement = steps * step_size * scale_factor;  // Total movement in mm (positive or negative)

        // Ensure movement is rounded to a reasonable precision (e.g., 3 decimal places for mm)
        movement = roundf(movement * 1000.0) / 1000.0;  // Round to 0.001 mm precision

        // Clamp movement to avoid tiny or zero movements (e.g., due to noise or rounding)
        if (fabs(movement) < 0.001) {
            return;  // Ignore movements smaller than 0.001 mm
        }

        // Generate G-Code in the format $J=G91 G21 [Axis][Distance] F1000
        // Use %.3f for float formatting to ensure 3 decimal places (e.g., 0.001, 0.010, 0.100)
        int len = snprintf(gcode, sizeof(gcode), "$J=G91 G21 %c%.3f F1000\r\n", selected_axis, movement);

        if (len < 0 || len >= sizeof(gcode)) {
            printk("G-Code formatting error\n");
            return;
        }

        printk("Generated G-Code: %s", gcode);  // Debug output to verify
        printk("Debug - steps: %d, scale_factor: %d, movement: %.3f\n",
            steps, scale_factor, movement);  // Additional debugging

        fnc_send_line(gcode, 10);
        // uart_poll_out(uart_dev, gcode, strlen(gcode));
    }

    void update_switches(void) {
        if (!gpio_pin_get_dt(&axis_x)) selected_axis = 'X';
        else if (!gpio_pin_get_dt(&axis_y)) selected_axis = 'Y';
        else if (!gpio_pin_get_dt(&axis_z)) selected_axis = 'Z';
        else selected_axis = 'O';

        if (!gpio_pin_get_dt(&scale_x1)) scale_factor = 1;    // x1
        else if (!gpio_pin_get_dt(&scale_x10)) scale_factor = 10; // x10
        else if (!gpio_pin_get_dt(&scale_x100)) scale_factor = 100; // x100
    }

public:
	FluidNCController() {
		if (!init_uart()) {
			printk("Failed to initialize UART");
		}
	}

	void run() {
        fnc_wait_ready();
        printk("Starting sequence\n");

        while(1) {
            update_switches();

            if (encoder_pos != 0 && selected_axis != 'O') {
                generate_gcode(encoder_pos);
                encoder_pos = 0;  // Reset after processing
            }
            
            k_msleep(10);
        }
	}
};

extern "C" {
	static const struct device *uart_dev_global = DEVICE_DT_GET(UART_DEVICE);

	int fnc_getchar() {
		unsigned char c;
		if (uart_poll_in(uart_dev_global, &c) == 0) {
			return c;
		}
		return -1;
	}

	void fnc_putchar(uint8_t c) {
		uart_poll_out(uart_dev_global, c);
	}

	int milliseconds() {
		return k_uptime_get_32();
	}

	void show_state(const char *state) {
		printk("State: %s\n", state);
	}
}


int main() {
	int ret; 
	/* Initialize GPIO pins */
    if (!device_is_ready(encoder_a.port)) {
        printk("Encoder A GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(encoder_b.port)) {
        printk("Encoder B GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(encoder_a_inv.port)) {
        printk("Encoder A_INV GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(encoder_b_inv.port)) {
        printk("Encoder B_INV GPIO device not ready\n");
        return -1;
    }
	if (!device_is_ready(axis_x.port)) {
        printk("Axis X GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(axis_y.port)) {
        printk("Axis Y GPIO device not ready\n");
        return -1;
    }
	if (!device_is_ready(axis_z.port)) {
        printk("Axis Z GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(scale_x1.port)) {
        printk("Scale X1 GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(scale_x10.port)) {
        printk("Scale X10 GPIO device not ready\n");
        return -1;
    }
    if (!device_is_ready(scale_x100.port)) {
        printk("Scale X100 GPIO device not ready\n");
        return -1;
    }

	ret = gpio_pin_configure_dt(&encoder_a, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure Encoder A pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&encoder_b, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure Encoder B pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&encoder_a_inv, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure Encoder A_INV pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&encoder_b_inv, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure Encoder B_INV pin\n");
        return -1;
    }
	ret = gpio_pin_configure_dt(&axis_x, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Axis X pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&axis_y, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Axis Y pin\n");
        return -1;
    }
	ret = gpio_pin_configure_dt(&axis_z, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Axis Z pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&scale_x1, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Scale X1 pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&scale_x10, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Scale X10 pin\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&scale_x100, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure Scale X100 pin\n");
        return -1;
    }

	// Setup interrupts
	static struct gpio_callback encoder_cb;

	ret = gpio_pin_interrupt_configure_dt(&encoder_a, GPIO_INT_EDGE_BOTH);
    if (ret < 0) {
        printk("Failed to configure Encoder A interrupt\n");
        return -1;
    }
    ret = gpio_pin_interrupt_configure_dt(&encoder_b, GPIO_INT_EDGE_BOTH);
    if (ret < 0) {
        printk("Failed to configure Encoder B interrupt\n");
        return -1;
    }
    // ret = gpio_pin_interrupt_configure_dt(&encoder_a_inv, GPIO_INT_EDGE_BOTH);
    // if (ret < 0) {
    //     printk("Failed to configure Encoder A_INV interrupt\n");
    //     return -1;
    // }
    // ret = gpio_pin_interrupt_configure_dt(&encoder_b_inv, GPIO_INT_EDGE_BOTH);
    // if (ret < 0) {
    //     printk("Failed to configure Encoder B_INV interrupt\n");
    //     return -1;
    // }

	// gpio_init_callback(&encoder_cb, encoder_isr, BIT(encoder_a.pin) | BIT(encoder_b.pin));
	gpio_init_callback(&encoder_cb, encoder_isr, BIT(encoder_a.pin)); // | BIT(encoder_b.pin) | BIT(encoder_a_inv.pin) | BIT(encoder_b_inv.pin));
    gpio_add_callback(encoder_a.port, &encoder_cb);
	
    printk("CNC Pendant Test! %s\n", CONFIG_BOARD_TARGET);    

    FluidNCController controller;
    controller.run();
}
