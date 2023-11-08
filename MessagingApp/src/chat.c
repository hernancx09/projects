#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mq/string.h"
#include "mq/client.h"

#define StartTopic "GLOBAL"

void *incoming_thread(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;

    while (!mq_shutdown(mq)) {
        char *message = mq_retrieve(mq);
        if (message) {
            printf("\r%s\n", message);
            free(message);
        }
    }

    return NULL;
}

int main() {
    char username[BUFSIZ];
    printf("Enter your username: ");
    fgets(username, BUFSIZ, stdin);
    username[strlen(username) - 1] = '\0'; // Remove newline character

    MessageQueue *mq = mq_create(username, "localhost", "9876");
    
    mq_subscribe(mq, StartTopic);
    mq_unsubscribe(mq, StartTopic);
    mq_subscribe(mq,StartTopic);
    mq_start(mq);
    printf("1) to send a mesage type /M *message*\n2) to end the chat type /exit\n");
    Thread incoming;
    thread_create(&incoming, NULL, incoming_thread, mq);

    char input[BUFSIZ];

    while (1) {
        fgets(input, BUFSIZ, stdin);
        input[strlen(input) - 1] = '\0'; // Remove newline character
        if (streq(input, "/exit")) {
			break;
		}

        if (strstr(input, "/M")) {
    		char *message = input + 3; // Skip "/M " to get the message
            char finalMSG[BUFSIZ] = "";
            while (*message == ' ') {  // Skip any leading spaces
        		message++;
    		}
            strcat(finalMSG, username);
            strcat(finalMSG, ": ");
            strcat(finalMSG,input + 3);
            mq_publish(mq, StartTopic, finalMSG);


        }
    }


    mq_stop(mq);
    mq_unsubscribe(mq, StartTopic);
    thread_join(incoming, NULL);
    sleep(2);
    mq_delete(mq);


    return 0;
}