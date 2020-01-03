#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>

template <typename T> struct node {
    struct node* next;
    T data;
};

template <typename T> class Queue {
  private:
    struct node<T>* head = NULL;
    struct node<T>* tail = NULL;

  public:
    Queue() {
    }
    ~Queue() {
        while (head != NULL) {
            struct node<T>* next = head->next;
            delete head;
            head = next;
        }
    }
    void enqueue(T data) {
        struct node<T>* node = new struct node<T>;
        node->next = NULL;
        node->data = data;

        if (head == NULL) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    T dequeue() {
        struct node<T>* next = head->next;
        T data = head->data;

        delete head;
        head = next;

        return data;
    }
    bool empty() {
        return head == NULL;
    }
};

#endif /* end of include guard: LINKED_LIST_H */
