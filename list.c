#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "list.h"

node_t * createNode(int socket_fd) {
  node_t* cur = (node_t*) malloc(sizeof(node_t));
  cur->socket_fd = socket_fd;
  pthread_mutex_init(&(cur->node_m), NULL);
  cur->next = NULL;
  return cur;
}

linkedlist_t * listInit() {
  linkedlist_t* cur = (linkedlist_t*) malloc(sizeof(linkedlist_t));
  cur->first = NULL;
  pthread_mutex_init(&(cur->lst_m), NULL);
  return cur;
}

void addNode(linkedlist_t* list, node_t* node) {
  pthread_mutex_lock(&list->lst_m);
  if (list->first == NULL)
    list->first = node;
  else {
    node_t* cur = list->first;
    while (cur->next != NULL) {
      cur = cur->next;
    }

    cur->next = node;
  }
  list->counter++;
  pthread_mutex_unlock(&list->lst_m);
}

void removeNode(linkedlist_t* list, int socket_fd) {
  pthread_mutex_lock(&list->lst_m);
  if (list->first == NULL) {
    pthread_mutex_unlock(&list->lst_m);
    return;
  } else {
    node_t* cur = list->first;
    node_t* prev = list->first;
    while (cur != NULL &&  cur->socket_fd != socket_fd) {
      prev = cur;
      cur = cur->next;
    }
    if (cur != NULL) {
      prev->next = cur->next;
      free(cur);
    }
  }
  list->counter--;
  pthread_mutex_unlock(&list->lst_m);
}

void destroyList(linkedlist_t* list) {
  pthread_mutex_lock(&list->lst_m);
  node_t* cur = list->first;
  while(cur != NULL) {
    node_t * temp = cur;
    cur = cur->next;
    free(temp);
  }
  pthread_mutex_unlock(&list->lst_m);
  free(list);
}

