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

class rb_tree_node {
  public:
    rb_tree_node(te_sptr te){ n_te = te; }
    ~rb_tree_node(){}
    inline void set_color(COLOR clr){ n_color = clr; }
    inline COLOR color(void){ return n_color; }
    inline bool is_black(void){ return n_color == COLOR::BLACK;}
    inline bool is_red(void){ return n_color == COLOR::RED;}
    inline rbt_node_wptr left(void){ return n_left; }
    inline void set_left(rbt_node_sptr nd){ n_left = nd; }
    inline rbt_node_wptr right(void){ return n_right; }
    inline void set_right(rbt_node_sptr nd){ n_right = nd; }
    inline rbt_node_wptr parent(void){ return n_parent; }
    inline void set_parent(rbt_node_wptr nd){ n_parent = nd; }

    inline rb_tree_key_t key(void){ return n_key; }
    inline void set_key(rb_tree_key_t key){ n_key = key; }
    inline u_char data(void){ return n_data; }
    inline void set_data(u_char data){ n_data = data; }
    inline te_wptr te(void){ return n_te; }
  private:
    rb_tree_key_t n_key = 0;
    rbt_node_sptr n_left;
    rbt_node_sptr n_right;
    rbt_node_wptr n_parent;
    COLOR         n_color = COLOR::BLACK;
    u_char        n_data = 0;
    te_sptr       n_te;
  };

  class rb_tree {
  public:

    rb_tree();
    ~rb_tree();

    void insert_node(rbt_node_sptr nd);
    void delete_node(rbt_node_sptr nd);
    inline bool is_sentinel(rbt_node_wptr nd){ return nd.lock() == sentinel; }
    inline bool is_root(rbt_node_wptr nd){ return nd.lock() == root; }
    inline bool is_empty(){ return root == sentinel; }
    rbt_node_wptr get_min(void);

  private:
    void left_rotate(rbt_node_wptr nd);
    void right_rotate(rbt_node_wptr nd);
    void insert_timer_value(rbt_node_sptr nd);
    rbt_node_wptr get_min_of_node(rbt_node_wptr nd);

  private:
    rbt_node_sptr root;
    rbt_node_sptr sentinel;
  };

  class timer_event {
  public:
    timer_event();
    ~timer_event();
    inline TE_HANDLER handler(void){ return te_handler; }
    inline void set_handler(TE_HANDLER handler){ te_handler = handler; }
    inline msec_t timer(void){ return static_cast<msec_t>(node.lock()->key()); }
    inline void set_timer(msec_t timer){
                          node.lock()->set_key(static_cast<rb_tree_key_t>(timer));
                        }
    inline rbt_node_wptr rbt_node(void){ return node; }
    inline void set_rbt_node(rbt_node_wptr nd){ node = nd; }
  private:
    timer_event(const timer_event&){}
    const timer_event& operator=(const timer_event&){ return *this; }
  private:
    TE_HANDLER te_handler;
    rbt_node_wptr node;
  };

  class timer{
  public:
    timer();
    ~timer();

    static msec_t get_cur_msec(void);
    msec_t find_timer(void);
    te_wptr add_timer(msec_t timer, TE_HANDLER handler);
    void del_timer(te_wptr te);
    void expire_timers(msec_t cur_timer);
  private:
    timer(const timer &){}
    const timer& operator=(const timer& t){ if(this != &t){} return *this; }
  private:
    rb_tree timer_rb_tree;
  };

}

#endif //_XEL_TIMER_H_INCLUDED_
