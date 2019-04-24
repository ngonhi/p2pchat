#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifndef __LIST_H__
#define __LIST_H__

typedef struct node {
  int socket_fd;
  const char* username;
  pthread_mutex_t node_m;
  struct node* next;
} node_t;

typedef struct linkedlist {
  pthread_mutex_t lst_m;
  node_t * first;
  int counter;
} linkedlist_t;

node_t * createNode(int socket_fd);

linkedlist_t * listInit();

void addNode(linkedlist_t* list, node_t* node);

void removeNode(linkedlist_t* list, int socket_fd);

void destroyList(linkedlist_t* list);

#endif
