#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "ui.h"

// Keep the username in a global so we can access it from the callback
const char* username;

int counter = 0;

unsigned short port;

typedef struct header {
  int message_length;
  int username_length;
} header_t;

typedef struct node {
  const char* username;
  int socket_fd;
  struct node *next;
} node_t;

node_t* head = NULL;

node_t* createNode(node_t* head) {
  node_t* temp;
  temp = (node_t*)malloc(sizeof(node_t));
  temp->next = NULL;
  temp->username = username;
  return temp;
}

node_t* addNode(node_t* head, int value) {
  node_t* temp, p;
  temp = createNode(head);
  temp->socket_fd = value; 
  if(head == NULL){
    head = temp;
    counter++;
  }
  else{
    p  = *head;
    while(p.next != NULL){
      p = *(p.next);
    }
    p.next = temp;
    counter++;
  }
  printf("counter: %d\n", counter);
  return head;
}

typedef struct args {
  const char* message;
  int socket_fd;
} args_t;

void* message_reader(void* args) {
  int port = (int)args;
  header_t* h = malloc(sizeof(header_t));
  const char* message;
  const char* peer_user;
  read(port, &h, sizeof(header_t));
  read(port, &peer_user, h->username_length);
  read(port, &message, h->message_length);
  ui_display(peer_user, message);
  return NULL;
}

void* message_writer(void* args) {
  args_t* p = args;
  header_t* h = malloc(sizeof(header_t));
  h->username_length = strlen(username);
  h->message_length = strlen(p->message);
  write(p->socket_fd, &h, sizeof(header_t));
  write(p->socket_fd, &username, strlen(username));
  write(p->socket_fd, p->message, strlen(p->message));
    ui_display(username, p->message);
  return NULL;
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if(strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    pthread_t threads[counter];
    args_t args[counter];
    node_t* p = head;
    int i = 0;
    ui_display(p->username, "HI");
    while(p->next != NULL) {
      
      if(strcmp(p->username, username) != 0) {
        args[i].message = message;
        args[i].socket_fd = p->socket_fd;
        if(pthread_create(&threads[i], NULL, message_writer, &args[i])) {
          perror("pthread_create failed");
          exit(2);
        }
      }
      p = p->next;
      i++;
    }
    ui_display(username, message);
    pthread_t threads1[counter];
    node_t* p2 = head;
    int m = 0;
      while(p2->next != NULL) {
        if(strcmp(p2->username, username) != 0) {
          if(pthread_create(&threads1[m], NULL, message_reader, &p2->socket_fd)) {
            perror("pthread_create failed");
            exit(2);
          }
        }
        p2 = p2->next;
        m++;

    }
  }
}

int main(int argc, char** argv) {
  // Make sure the arguments include a username
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }
 
  // Save the username in a global
  username = argv[1];

  unsigned short port;
  if(argc == 2) {
    // Open server socket
    port = 0;
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
      perror("Server socket was not opened.\n");
      exit(2);
    }
    // Listen to server socket
    if(listen(server_socket_fd, 1)) {
      perror("Listen failed.\n");
      exit(2);
    }

    addNode(head, server_socket_fd);
    
  }
  //threads
  
  // Did the user specify a peer we should connect to?
  if(argc == 4) {
    // Unpack arguments
    char* peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);
    
    // Connect peer to server socket
    int peer_socket_fd = socket_connect(peer_hostname, peer_port);
    if (peer_socket_fd == -1) {
      perror("Server socket was not opened.\n");
      exit(2);
    }

    //save info to data structure (Is it the fd or the peer port that we want?)
    
    addNode(head, peer_socket_fd);
  }


  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);
  
  // Keep reading for messages from all sockets available
  
  char port_string[20];
  snprintf(port_string, 20, "%d", port);
  // Once the UI is running, you can use it to display log messages
  ui_display(username, port_string);
  
  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

return 0;
}

//To do: print username and port when connecting
//Make sure it works
