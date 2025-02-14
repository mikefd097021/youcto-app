#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Structure definitions
typedef struct {
    GtkWidget *label;
    const char *command;
} ButtonCallbackData;

typedef struct {
    const char *name;
    const char *command;
    GtkWidget *button;
    GtkWidget *label;
    ButtonCallbackData *callback_data;
} TestButton;

// Constants
#define MAX_BUFFER_SIZE 512
#define MAX_PATH_SIZE 50
#define MAX_COMMAND_SIZE 100
#define GPIO_CYCLE_COUNT 10
#define GPIO_DELAY_SEC 1

// GPIO-related functions
static void export_gpio(int gpio_id) {
    char command[MAX_COMMAND_SIZE];
    snprintf(command, sizeof(command), "echo %d > /sys/class/gpio/export", gpio_id);
    system(command);
}

static void set_gpio_direction(int gpio_id, const char *direction) {
    char command[MAX_COMMAND_SIZE];
    snprintf(command, sizeof(command), "echo %s > /sys/class/gpio/gpio%d/direction", direction, gpio_id);
    system(command);
}

static void set_gpio_value(int gpio_id, int value) {
    char command[MAX_COMMAND_SIZE];
    snprintf(command, sizeof(command), "echo %d > /sys/class/gpio/gpio%d/value", value);
    system(command);
}

// GPIO control function
void control_gpio(int gpio_ids[], int size) {
    for (int cycle = 1; cycle <= GPIO_CYCLE_COUNT; cycle++) {
        printf("Cycle %d: Setting all GPIOs to 1\n", cycle);

        for (int i = 0; i < size; i++) {
            char path[MAX_PATH_SIZE];
            snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", gpio_ids[i]);
            
            if (access(path, F_OK) != 0) {
                export_gpio(gpio_ids[i]);
            }
            
            set_gpio_direction(gpio_ids[i], "out");
            set_gpio_value(gpio_ids[i], 1);
        }

        sleep(GPIO_DELAY_SEC);
        printf("Cycle %d: Setting all GPIOs to 0\n", cycle);

        for (int i = 0; i < size; i++) {
            set_gpio_value(gpio_ids[i], 0);
        }

        sleep(GPIO_DELAY_SEC);
    }

    printf("All GPIO cycles are complete!\n");
}

// Callback functions
static void on_gpio_button_clicked(GtkWidget *widget, gpointer user_data) {
    int gpio_ids[] = {5, 6, 8, 13, 16, 17, 90, 91};
    int size = sizeof(gpio_ids) / sizeof(gpio_ids[0]);
    control_gpio(gpio_ids, size);
}

static void on_scale_value_changed(GtkRange *range, gpointer user_data) {
    int value = (int)gtk_range_get_value(range);
    char command[MAX_COMMAND_SIZE];
    snprintf(command, sizeof(command), "echo %d > /sys/class/backlight/lvds-backlight/brightness", value);
    system(command);
    printf("Brightness set to: %d\n", value);
}

static void on_test_button_clicked(GtkWidget *widget, gpointer user_data) {
    ButtonCallbackData *data = (ButtonCallbackData *)user_data;
    char buffer[MAX_BUFFER_SIZE];
    char result[MAX_BUFFER_SIZE] = {0};
    FILE *fp;

    fp = popen(data->command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        gtk_label_set_text(GTK_LABEL(data->label), "Failed to execute command.");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(result, buffer, sizeof(result) - strlen(result) - 1);
    }
    pclose(fp);

    gtk_label_set_text(GTK_LABEL(data->label), result);
}

// Helper function to create test button
static TestButton* create_test_button(const char *name, const char *command) {
    TestButton *test = g_malloc(sizeof(TestButton));
    test->name = name;
    test->command = command;
    test->callback_data = g_malloc(sizeof(ButtonCallbackData));
    
    test->button = gtk_button_new_with_label(name);
    gtk_widget_set_size_request(test->button, 300, 50);
    
    test->label = gtk_label_new("Test results will be shown here.");
    
    test->callback_data->label = test->label;
    test->callback_data->command = command;
    
    g_signal_connect(test->button, "clicked", G_CALLBACK(on_test_button_clicked), test->callback_data);
    
    return test;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    // Create vertical container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create brightness scale
    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_TOP);
    gtk_scale_set_digits(GTK_SCALE(scale), 0);
    gtk_widget_set_size_request(scale, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), scale, FALSE, FALSE, 10);
    g_signal_connect(scale, "value-changed", G_CALLBACK(on_scale_value_changed), NULL);

    // Create GPIO control button
    GtkWidget *gpio_button = gtk_button_new_with_label("Start GPIO Control");
    gtk_widget_set_size_request(gpio_button, 300, 50);
    gtk_box_pack_start(GTK_BOX(vbox), gpio_button, FALSE, FALSE, 10);
    g_signal_connect(gpio_button, "clicked", G_CALLBACK(on_gpio_button_clicked), NULL);

    // Create test buttons
    TestButton *tests[] = {
        create_test_button("eMMC Test", "dd if=/dev/zero of=/tmp/test bs=1M count=500 conv=fsync 2>&1"),
        create_test_button("UART Test", "fltest_uarttest -d /dev/ttyS3"),
        create_test_button("SPI Test", "fltest_spidev_test -D /dev/spidev0.0"),
        create_test_button("KEY Test", "fltest_keytest")
    };

    // Add test buttons and labels to vbox
    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        gtk_box_pack_start(GTK_BOX(vbox), tests[i]->button, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(vbox), tests[i]->label, FALSE, FALSE, 10);
    }

    gtk_widget_show_all(window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();

    // Cleanup
    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        g_free(tests[i]->callback_data);
        g_free(tests[i]);
    }

    return 0;
}

