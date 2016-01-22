/*
 *
 */

#ifndef _XEL_TIMER_H_INCLUDED_
#define _XEL_TIMER_H_INCLUDED_

#include <stdint.h>
#include <functional>
#include "xel.h"

namespace xel {
  // class rb_tree;
  // class rb_tree_node;
  // typedef msec_t rb_tree_key_t;
  // typedef enum {BLACK, RED} COLOR;

  class timer{
  public:
    timer();
    ~timer();
  private:
    timer(const timer &){}
    const timer& operator=(const timer& t){ if(this != &t){} return *this; }
  private:
    rb_tree *timer_rb_tree;
  };

  class rb_tree_node {
  public:
    rb_tree_node();
    ~rb_tree_node();
    inline void set_color(COLOR clr){ n_color = clr; }
    inline COLOR color(void){ return n_color; }
    inline bool is_black(void){ return n_color == COLOR::BLACK;}
    inline bool is_red(void){ return n_color == COLOR::RED;}
    inline rb_tree_node* left(void){ return n_left; }
    inline void set_left(rb_tree_node *nd){ n_left = nd; }
    inline rb_tree_node* right(void){ return n_right; }
    inline void set_right(rb_tree_node *nd){ n_right = nd; }
    inline rb_tree_node* parent(void){ return n_parent; }
    inline void set_parent(rb_tree_node *nd){ n_parent = nd; }
    inline rb_tree_key_t key(void){ return n_key; }
    inline void set_key(rb_tree_key_t key){ n_key = key; }
    inline u_char data(void){ return n_data; }
    inline void set_data(u_char data){ n_data = data; }
  private:
    rb_tree_key_t  n_key = 0;
    rb_tree_node *n_left = nullptr;
    rb_tree_node *n_right = nullptr;
    rb_tree_node *n_parent = nullptr;
    COLOR          n_color = COLOR::BLACK;
    u_char         n_data = 0;

  };

  class rb_tree {
  public:

    rb_tree();
    ~rb_tree();

    void insert_node(rb_tree_node *nd);
    void delete_node(rb_tree_node *nd);
    inline bool is_sentinel(rb_tree_node *nd){ return nd == sentinel; }
    inline bool is_root(rb_tree_node *nd){ return nd == root; }
    inline bool is_empty(){ return root == sentinel; }
  private:
    void left_rotate(rb_tree_node *nd);
    void right_rotate(rb_tree_node *nd);
    rb_tree_node *get_min(rb_tree_node *nd);
    void insert_timer_value(rb_tree_node *nd);
  private:
    rb_tree_node *root;
    rb_tree_node *sentinel;
  };

}

#endif //_XEL_TIMER_H_INCLUDED_
