#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "ui.h"
#include "list.h"

// Keep the username in a global so we can access it from the callback
const char* username;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

unsigned short port;

typedef struct header {
  int message_length;
  int username_length;
} header_t;

linkedlist_t* list = NULL;

typedef struct args {
  const char* message;
  int socket_fd;
} args_t;

void* message_writer(const char* username, const char* message) {
  header_t* h = malloc(sizeof(header_t));
  h->username_length = strlen(username);
  h->message_length = strlen(message);

  node_t* current = list->first;
  while(current != NULL) {
    if (strcmp(current->username, username) != 0) {
      write(current->socket_fd, &h, sizeof(header_t));
      write(current->socket_fd, &username, h->username_length);
      write(current->socket_fd, message, h->message_length);
    }
    current = current->next;
  }
  return NULL;
}

void* message_reader(void* p) {
  args_t* args = p;
  int socket_fd = args->socket_fd;
  while (1) {
    header_t h;
    if (read(socket_fd, &h, sizeof(header_t)) != sizeof(header_t)) {
      removeNode(list, socket_fd);
      close(socket_fd);
      break;
    }

    char* username = (char*)malloc(h.username_length + 1);
    char* message = (char*)malloc(h.message_length + 1);

    if (read(socket_fd, username, h.username_length) != h.username_length) {
      removeNode(list, socket_fd);
      close(socket_fd);
      break;
    }

    if (read(socket_fd, message, h.message_length) != h.message_length) {
      removeNode(list, socket_fd);
      close(socket_fd);
      break;
    }

    username[h.username_length + 1] = '\0';
    message[h.message_length + 1] = '\0';
    
    ui_display(username, message);

    // Send message to everyone
    message_writer(username, message);
  }
  // Read socket header, if not exist, close socket
  // Read username
  // Read message
  // Display message
  return NULL;
}

// Send to everyone in linkedlist accept for us, keep file_descriptor
// loop through neighbors and send the header
// close file neighbor
// unlock

void* server_thread_fn(void* p) {
  args_t* args = p;
  int server_socket_fd = args->socket_fd;
  while(1) {
    pthread_mutex_lock(&m);
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if(client_socket_fd == -1) {
      perror("accept failed");
      pthread_mutex_unlock(&m);
      exit(2);
    }
    node_t* current = createNode(client_socket_fd);
    addNode(list, current);
    printf("\ncounter %d\n", list->counter);
    printf("\nClient connected!\n");
  }

  close(server_socket_fd);
  pthread_mutex_unlock(&m);

  return NULL;
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if(strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    ui_display(username, message);
    message_writer(username, message);
  }
}

int main(int argc, char** argv) {
  list = listInit();
  char port_string[20];

  // Make sure the arguments include a username
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }
 
  // Save the username in a global
  username = argv[1];

  unsigned short port;
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

  node_t* firstNode = createNode(server_socket_fd);
  firstNode->username = username;
  addNode(list, firstNode);
  snprintf(port_string, 20, "%d", port);

  // Keep on waiting for connection
  args_t args;
  args.socket_fd = server_socket_fd;
  pthread_t thread;
  if(pthread_create(&thread, NULL, server_thread_fn, &args)) {
    perror("pthread_create failed");
    exit(2);
  }

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
    node_t* curNode = createNode(peer_socket_fd);
    curNode->username = username;
    addNode(list, curNode);
    snprintf(port_string, 20, "%d", peer_port);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Once the UI is running, you can use it to display log messages
  ui_display(username, port_string);
  
  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

return 0;
}


// write read and connect
