#include "header.h"

/* @tehu command functionality - prints temperature or humidity data.
 * 1. Suspend task dht11_monitor in order it can`t write in queue.
 * 2. Receive all data from queue and store it in buffer data.
 * 3. Print data depending on flags in CLI and send it back in Queue.
 * 4. Resume task dht11_monitor.
 */



// Prints current temperature and humidity.
static void print_current_tehu(char **data) {
    int len = mx_strarr_len(data);
    uart_write_bytes(UART_PORT, NEWLINE, strlen(NEWLINE));
    uart_write_bytes(UART_PORT, (const char*)data[len - 1], strlen((const char*)data[len -1]));
}


/*
 * Prints all temperature/humidity values stored in queue (dht11_data_queue)
 * if temperature or humidity value has changed, also prints how long ago it
 * has changed.
 */
static void print_full_log(char **data) {
    char *previous_value = NULL;
    char *tmp = NULL;
    char time_buff[100];
    int seconds_passed;

    uart_write_bytes(UART_PORT, NEWLINE, strlen(NEWLINE));
    for (int i = 0; data[i]; ++i) {
        bzero(time_buff, 100);
        seconds_passed =  (mx_strarr_len(data) * 5) - (i * 5);
        sprintf(time_buff, 
            " |\e[35mvalues changed \e[31m%d\e[35mmin \e[31m%d\e[35msec ago\e[0m", 
            seconds_passed / 60, seconds_passed % 60);
        if (i != 0) 
            uart_write_bytes(UART_PORT, NEWLINE, strlen(NEWLINE));
        uart_write_bytes(UART_PORT, (const char*)data[i], strlen((const char*)data[i]));
        if (previous_value && strcmp(previous_value, data[i]) != 0)
            uart_write_bytes(UART_PORT, (const char*)time_buff, strlen((const char*)time_buff));

        if (previous_value)
            tmp = previous_value;
        previous_value = mx_string_copy(data[i]);
        if (tmp) 
            free(tmp);
    }
}




static void tehu_syntax_error() {
    char *msg = "\e[31msyntax error:\e[0m tehu [-f]";
    uart_write_bytes(UART_PORT, msg, strlen((const char*)msg));
}


/*
 * Receives all data form queue(dht11_data_queue) and stores it in buffer(data)
 * returns buffer.
 */
static char **data_from_queue_receive() {
    char buff[100];
    char **data = mx_strarr_new(60);
    int index = 0;

    while(1) {
        bzero(buff, 70);
        xQueueReceive(dht11_data_queue, (void *)buff, (TickType_t)0);
        if (strlen((char *)buff) == 0) break;
        data[index] = mx_string_copy(buff);
        index++;
    }
    return data;
}



static void tehu_execute(char **cmd, char **data) {
    if (mx_strarr_len(cmd) == 1 && cmd[1] == NULL)
        print_current_tehu(data);
    else if (mx_strarr_len(cmd) == 2 && !strcmp(cmd[1], "-f"))
        print_full_log(data);
    else
        tehu_syntax_error();
    uart_write_bytes(UART_PORT, NEWLINE, strlen(NEWLINE));
}



static void sendback_data_in_queue(char **data) {
    for (int i = 0; data[i]; ++i) {
        xQueueSend(dht11_data_queue,(void *)data[i], (TickType_t)0);
        free(data[i]);
    }  
}

/* @ Prints dht11 log in CLI.
 * 1. Suspend task dht11_monitor in order it can`t write in queue.
 * 2. Receive all data from queue and store it in buffer data.
 * 3. Print data in CLI and send it back in Queue.
 * 4. Resume task dht11_monitor.
 */
void dht11_log(char **cmd) {
    char **data = NULL;

    vTaskSuspend(xTaskWeather);
    if (dht11_data_queue != 0) {
        data = data_from_queue_receive();
        if (data == NULL) return;
        tehu_execute(cmd, data);
        sendback_data_in_queue(data);
        free(data);
    }
    vTaskResume(xTaskWeather);
}

