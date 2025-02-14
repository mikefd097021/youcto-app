#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    GtkWidget *label;        // Label for displaying results
    const char *command;     // Test command
} ButtonCallbackData;

// GPIO control function
void control_gpio(int gpio_ids[], int size) {
    for (int cycle = 1; cycle <= 10; cycle++) {
        printf("Cycle %d: Setting all GPIOs to 1\n", cycle);

        // Set all GPIOs to 1
        for (int i = 0; i < size; i++) {
            char path[50], command[100];

            // Check if GPIO is exported
            snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", gpio_ids[i]);
            if (access(path, F_OK) != 0) {
                snprintf(command, sizeof(command), "echo %d > /sys/class/gpio/export", gpio_ids[i]);
                system(command);
            }

            // Set direction to output
            snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio_ids[i]);
            snprintf(command, sizeof(command), "echo out > %s", path);
            system(command);

            // Set value to 1
            snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio_ids[i]);
            snprintf(command, sizeof(command), "echo 1 > %s", path);
            system(command);
        }

        sleep(1);

        printf("Cycle %d: Setting all GPIOs to 0\n", cycle);

        // Set all GPIOs to 0
        for (int i = 0; i < size; i++) {
            char path[50], command[100];
            snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio_ids[i]);
            snprintf(command, sizeof(command), "echo 0 > %s", path);
            system(command);
        }

        sleep(1);
    }

    printf("All GPIO cycles are complete!\n");
}

// Button callback function - GPIO control
void on_gpio_button_clicked(GtkWidget *widget, gpointer user_data) {
    int gpio_ids[] = {5, 6, 8, 13, 16, 17, 90, 91};
    int size = sizeof(gpio_ids) / sizeof(gpio_ids[0]);

    control_gpio(gpio_ids, size);
}

// Scale callback function - Adjust brightness
void on_scale_value_changed(GtkRange *range, gpointer user_data) {
    int value = (int)gtk_range_get_value(range);
    char command[100];

    snprintf(command, sizeof(command), "echo %d > /sys/class/backlight/lvds-backlight/brightness", value);
    system(command);

    printf("Brightness set to: %d\n", value);
}

// Button callback function - eMMC test
void on_emmc_test_button_clicked(GtkWidget *widget, gpointer label) {
    char buffer[256];
    char result[512] = {0};
    FILE *fp;

    // Execute eMMC test command
    fp = popen("dd if=/dev/zero of=./test bs=1M count=500 conv=fsync 2>&1", "r");
    if (fp == NULL) {
        perror("Failed to run dd command");
        return;
    }

    // Read output and store
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(result, buffer, sizeof(result) - strlen(result) - 1);
    }
    pclose(fp);

    // Display test results
    gtk_label_set_text(GTK_LABEL(label), result);
}

void on_test_button_clicked(GtkWidget *widget, gpointer user_data) {
    char buffer[256];
    char result[512] = {0};
    FILE *fp;

    // Unpack callback data
    ButtonCallbackData *data = (ButtonCallbackData *)user_data;
    const char *command = data->command;
    GtkWidget *label = data->label;

    // Execute the test command
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        gtk_label_set_text(GTK_LABEL(label), "Failed to execute command.");
        return;
    }

    // Read output and store
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(result, buffer, sizeof(result) - strlen(result) - 1);
    }
    pclose(fp);

    // Display test results
    gtk_label_set_text(GTK_LABEL(label), result);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *scale;
    GtkWidget *gpio_button;
    GtkWidget *emmc_button;
    GtkWidget *emmc_label;
    const char *emmc_command = "dd if=/dev/zero of=/tmp/test bs=1M count=500 conv=fsync 2>&1";
    ButtonCallbackData *emmc_callback_data = g_malloc(sizeof(ButtonCallbackData));
    GtkWidget *uart_button;
    GtkWidget *uart_label;
    const char *uart_command = "fltest_uarttest -d /dev/ttyS3";
    ButtonCallbackData *uart_callback_data = g_malloc(sizeof(ButtonCallbackData));
    GtkWidget *spi_button;
    GtkWidget *spi_label;
    const char *spi_command = "fltest_spidev_test -D /dev/spidev0.0";
    ButtonCallbackData *spi_callback_data = g_malloc(sizeof(ButtonCallbackData));
    GtkWidget *key_button;
    GtkWidget *key_label;
    const char *key_command = "fltest_keytest";
    //const char *key_command = "i2cdetect -y 1";
    ButtonCallbackData *key_callback_data = g_malloc(sizeof(ButtonCallbackData));


    gtk_init(&argc, &argv);

    // Create window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Remove title and border
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    // Create vertical container
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create scale
    scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_TOP);
    gtk_scale_set_digits(GTK_SCALE(scale), 0);
    gtk_widget_set_size_request(scale, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), scale, FALSE, FALSE, 10);

    // Register scale callback function
    g_signal_connect(scale, "value-changed", G_CALLBACK(on_scale_value_changed), NULL);

    // Create GPIO control button
    gpio_button = gtk_button_new_with_label("Start GPIO Control");
    gtk_widget_set_size_request(gpio_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), gpio_button, FALSE, FALSE, 10);

    // Register GPIO button callback
    g_signal_connect(gpio_button, "clicked", G_CALLBACK(on_gpio_button_clicked), NULL);

    // Create eMMC test button
    emmc_button = gtk_button_new_with_label("eMMC Test");
    gtk_widget_set_size_request(emmc_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), emmc_button, FALSE, FALSE, 10);

    // Create eMMC test result label
    emmc_label = gtk_label_new("eMMC test results will be shown here.");
    gtk_box_pack_start(GTK_BOX(vbox), emmc_label, FALSE, FALSE, 10);

    // Register eMMC button callback
    emmc_callback_data->label = emmc_label;
    emmc_callback_data->command = emmc_command;
    g_signal_connect(emmc_button, "clicked", G_CALLBACK(on_test_button_clicked), emmc_callback_data);

    // Create UART test button
    uart_button = gtk_button_new_with_label("UART Test");
    gtk_widget_set_size_request(uart_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), uart_button, FALSE, FALSE, 10);

    // Create UART test result label
    uart_label = gtk_label_new("UART test results will be shown here.");
    gtk_box_pack_start(GTK_BOX(vbox), uart_label, FALSE, FALSE, 10);

    // Register UART button callback
    uart_callback_data->label = uart_label;
    uart_callback_data->command = uart_command;
    g_signal_connect(uart_button, "clicked", G_CALLBACK(on_test_button_clicked), uart_callback_data);

    // Create SPI test button
    spi_button = gtk_button_new_with_label("SPI Test");
    gtk_widget_set_size_request(spi_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), spi_button, FALSE, FALSE, 10);

    // Create SPI test result label
    spi_label = gtk_label_new("SPI test results will be shown here.");
    gtk_box_pack_start(GTK_BOX(vbox), spi_label, FALSE, FALSE, 10);

    // Register SPI button callback
    spi_callback_data->label = spi_label;
    spi_callback_data->command = spi_command;
    g_signal_connect(spi_button, "clicked", G_CALLBACK(on_test_button_clicked), spi_callback_data);

    // Create KEY test button
    key_button = gtk_button_new_with_label("KEY Test");
    gtk_widget_set_size_request(key_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), key_button, FALSE, FALSE, 10);

    // Create KEY test result label
    key_label = gtk_label_new("KEY test results will be shown here.");
    gtk_box_pack_start(GTK_BOX(vbox), key_label, FALSE, FALSE, 10);

    // Register KEY button callback
    key_callback_data->label = key_label;
    key_callback_data->command = key_command;
    g_signal_connect(key_button, "clicked", G_CALLBACK(on_test_button_clicked), key_callback_data);

    gtk_widget_show_all(window);

    // Exit program when window is closed
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();

    return 0;
}

