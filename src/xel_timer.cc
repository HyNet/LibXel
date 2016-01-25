/*
 *
 */

#include <sys/time.h>
#include "xel_timer.h"

xel::timer::timer()
{
}

xel::timer::~timer()
{
}

xel::msec_t xel::timer::get_cur_msec(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  msec_t cur_msec = static_cast<msec_t>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
  return cur_msec;
}

xel::msec_t xel::timer::find_timer(void)
{
  rbt_node_wptr node;
  if (timer_rb_tree.is_empty()){
    // infinite time
    return -1;
  }

  node = timer_rb_tree.get_min();
  msec_t cur_msec = get_cur_msec();
  msec_t timer = node.lock()->key() - cur_msec;
  return (timer > 0 ? timer : 0);
}

xel::te_wptr xel::timer::add_timer(msec_t delta, TE_HANDLER handler)
{
  msec_t timer = get_cur_msec() + delta;
  te_sptr te = std::make_shared<timer_event>();
  rbt_node_sptr nd = std::make_shared<rb_tree_node>(te);
  te->set_rbt_node(nd);
  te->set_timer(timer);
  te->set_handler(handler);
  timer_rb_tree.insert_node(nd);
  return te;
}

void xel::timer::del_timer(te_wptr te)
{
  if (te.expired()){
    return;
  }

  rbt_node_wptr nd = te.lock()->rbt_node();
  timer_rb_tree.delete_node(nd.lock());
}

void xel::timer::expire_timers(msec_t cur_timer)
{
  rbt_node_wptr node;
  te_sptr te;

  for(;;){
    if (timer_rb_tree.is_empty()){
      return;
    }
    node = timer_rb_tree.get_min();
    if (node.expired()){
      return;
    }
    if(node.lock()->key() > cur_timer){
      return;
    }
    te = node.lock()->te().lock();
    timer_rb_tree.delete_node(node.lock());
    te->handler()();
  }

}

xel::timer_event::timer_event()
{
}

xel::timer_event::~timer_event()
{
}

xel::rb_tree::rb_tree()
{
  sentinel = std::make_shared<rb_tree_node>(nullptr);
  root = sentinel;
}

xel::rb_tree::~rb_tree()
{
}


void xel::rb_tree::left_rotate(rbt_node_wptr nd)
{
  if(nd.expired()){
    return;
  }
  rbt_node_wptr temp = nd.lock()->right();
  nd.lock()->set_right(temp.lock()->left().lock());
  if (!is_sentinel(temp.lock()->left())){
    temp.lock()->left().lock()->set_parent(nd);
  }

  temp.lock()->set_parent(nd.lock()->parent());
  if(is_root(nd)){
    root = temp.lock();
  } else if (nd.lock() == nd.lock()->parent().lock()->left().lock()){
    nd.lock()->parent().lock()->set_left(temp.lock());
  } else if (nd.lock() == nd.lock()->parent().lock()->right().lock()){
    nd.lock()->parent().lock()->set_right(temp.lock());
  }

  temp.lock()->set_left(nd.lock());
  nd.lock()->set_parent(temp);

}

void xel::rb_tree::right_rotate(rbt_node_wptr nd)
{
  rbt_node_wptr temp = nd.lock()->left();
  nd.lock()->set_left(temp.lock()->right().lock());
  if (!is_sentinel(temp.lock()->right())){
    temp.lock()->right().lock()->set_parent(nd);
  }

  temp.lock()->set_parent(nd.lock()->parent());
  if(is_root(nd)){
    root = temp.lock();
  } else if (nd.lock() == nd.lock()->parent().lock()->left().lock()){
    nd.lock()->parent().lock()->set_left(temp.lock());
  } else if (nd.lock() == nd.lock()->parent().lock()->right().lock()){
    nd.lock()->parent().lock()->set_right(temp.lock());
  }

  temp.lock()->set_right(nd.lock());
  nd.lock()->set_parent(temp.lock());
}

void xel::rb_tree::insert_timer_value(rbt_node_sptr nd)
{
  rbt_node_wptr temp = root;
  rbt_node_wptr p;
  for(;;){
    p = nd->key() < temp.lock()->key()? temp.lock()->left() : temp.lock()->right();
    if(is_sentinel(p)){
      break;
    }
    temp = p;
  }
  p = nd;
  nd->set_parent(temp);
  nd->set_left(sentinel);
  nd->set_right(sentinel);
  nd->set_color(COLOR::RED);
}

xel::rbt_node_wptr xel::rb_tree::get_min()
{
  return get_min_of_node(root);
}

xel::rbt_node_wptr xel::rb_tree::get_min_of_node(rbt_node_wptr nd)
{
  while(!is_sentinel(nd.lock()->left())){
    nd = nd.lock()->left();
  }
  return nd;
}
void xel::rb_tree::insert_node(rbt_node_sptr nd)
{
  /*
   * red black tree:
   * color of root node must be black
   */
  if(is_empty()){
    nd->set_parent(sentinel);
    nd->set_left(sentinel);
    nd->set_right(sentinel);
    nd->set_color(COLOR::BLACK);
    root = nd;
    return;
  }

  insert_timer_value(nd);
  /*
   * re-balance tree
   */
  rbt_node_wptr temp;

  while(!is_root(nd) && nd->parent().lock()->is_red()){
    if(nd->parent().lock() == nd->parent().lock()->parent().lock()->left().lock()){
      temp = nd->parent().lock()->parent().lock()->right();
      if (temp.lock()->is_red()){
        // both parent and parent's slibing is red, just re-color
        nd->parent().lock()->set_color(COLOR::BLACK);
        temp.lock()->set_color(COLOR::BLACK);
        nd->parent().lock()->parent().lock()->set_color(COLOR::RED);
        nd = nd->parent().lock()->parent().lock();
      } else {
        // parent is red and parent's slibing is not red
        if (nd == nd->parent().lock()->right().lock()){
          nd = nd->parent().lock();
          left_rotate(nd);
        }
        nd->parent().lock()->set_color(COLOR::BLACK);
        nd->parent().lock()->parent().lock()->set_color(COLOR::RED);
        right_rotate(nd->parent().lock()->parent());
      }

    } else {
      temp = nd->parent().lock()->parent().lock()->left();
      if (temp.lock()->is_red()){
        // both parent and parent's slibing is red, just re-color
        nd->parent().lock()->set_color(COLOR::BLACK);
        temp.lock()->set_color(COLOR::BLACK);
        nd->parent().lock()->parent().lock()->set_color(COLOR::RED);
        nd = nd->parent().lock()->parent().lock();
      } else {
        // parent is red and parent's slibing is not red
        if (nd == nd->parent().lock()->left().lock()){
          nd = nd->parent().lock();
          right_rotate(nd);
        }
        nd->parent().lock()->set_color(COLOR::BLACK);
        nd->parent().lock()->parent().lock()->set_color(COLOR::RED);
        left_rotate(nd->parent().lock()->parent());
      }
    }
  }
  root->set_color(COLOR::BLACK);
}
void xel::rb_tree::delete_node(rbt_node_sptr nd)
{
  rbt_node_wptr temp, sub;

  if(is_sentinel(nd->left())){
    temp = nd->right();
    sub = nd;
  } else if (is_sentinel(nd->right())){
    temp = nd->left();
    sub = nd;
  } else {
    // node is not leaf, get its right children's minimal node
    sub = get_min_of_node(nd->right());
    if(is_sentinel(sub.lock()->left())){
      temp = sub.lock()->left();
    } else {
      temp = sub.lock()->right();
    }
  }

  if(is_root(sub)){
    root = temp.lock();
    temp.lock()->set_color(COLOR::BLACK);
    return;
  }

  bool b_red = sub.lock()->is_red();

  if (sub.lock() == sub.lock()->parent().lock()->left().lock()){
    sub.lock()->parent().lock()->set_left(temp.lock());
  } else {
    sub.lock()->parent().lock()->set_right(temp.lock());
  }

  if (sub.lock() == nd){
    temp.lock()->set_parent(sub.lock()->parent().lock());
  } else {
    if (sub.lock()->parent().lock() == nd){
      temp.lock()->set_parent(sub);
    } else {
      temp.lock()->set_parent(sub.lock()->parent());
    }

    sub.lock()->set_parent(nd->parent());
    sub.lock()->set_left(nd->left().lock());
    sub.lock()->set_right(nd->right().lock());
    sub.lock()->set_color(nd->color());

    if(is_root(nd)){
      root = sub.lock();
    } else {
      if(nd == nd->parent().lock()->left().lock()){
        nd->parent().lock()->set_left(sub.lock());
      } else {
        nd->parent().lock()->set_right(sub.lock());
      }
    }

    if(!is_sentinel(sub.lock()->left())){
      sub.lock()->left().lock()->set_parent(sub);
    }

    if(!is_sentinel(sub.lock()->right())){
      sub.lock()->right().lock()->set_parent(sub);
    }
  }
  if(b_red){
    return;
  }

  /*
   * delete fixup
   */
  rbt_node_wptr w;

  while(!is_root(temp) && temp.lock()->is_black()){
    if(temp.lock() == temp.lock()->parent().lock()->left().lock()){
      w = temp.lock()->parent().lock()->right();
      if(w.lock()->is_red()){
        // node's color is black while parent's slibing is red, re-color
        w.lock()->set_color(COLOR::BLACK);
        temp.lock()->parent().lock()->set_color(COLOR::RED);
        left_rotate(temp.lock()->parent());
        w = temp.lock()->parent().lock()->right();
      }

      if(w.lock()->left().lock()->is_black()
           && w.lock()->right().lock()->is_black()){
        w.lock()->set_color(COLOR::RED);
        temp = temp.lock()->parent();
      } else {
        if(w.lock()->right().lock()->is_black()){
          w.lock()->left().lock()->set_color(COLOR::BLACK);
          w.lock()->set_color(COLOR::RED);
          right_rotate(w);
          w = temp.lock()->parent().lock()->right();
        }
        w.lock()->set_color(temp.lock()->parent().lock()->color());
        temp.lock()->parent().lock()->set_color(COLOR::BLACK);
        w.lock()->right().lock()->set_color(COLOR::BLACK);
        left_rotate(temp.lock()->parent());
        temp = root;
      }

    } else {
      w = temp.lock()->parent().lock()->left();
      if(w.lock()->is_red()){
        // node's color is black while parent's slibing is red, re-color
        w.lock()->set_color(COLOR::BLACK);
        temp.lock()->parent().lock()->set_color(COLOR::RED);
        right_rotate(temp.lock()->parent());
        w = temp.lock()->parent().lock()->left();
      }

      if(w.lock()->left().lock()->is_black()
           && w.lock()->right().lock()->is_black()){
        w.lock()->set_color(COLOR::RED);
        temp = temp.lock()->parent();
      } else {
        if(w.lock()->left().lock()->is_black()){
          w.lock()->right().lock()->set_color(COLOR::BLACK);
          w.lock()->set_color(COLOR::RED);
          left_rotate(w);
          w = temp.lock()->parent().lock()->left();
        }

        w.lock()->set_color(temp.lock()->parent().lock()->color());
        temp.lock()->parent().lock()->set_color(COLOR::BLACK);
        w.lock()->left().lock()->set_color(COLOR::BLACK);
        right_rotate(temp.lock()->parent());
        temp = root;
      }
    }
  }
  temp.lock()->set_color(COLOR::BLACK);
}

