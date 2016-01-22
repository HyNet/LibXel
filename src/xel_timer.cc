/*
 *
 */

#include "xel_timer.h"

xel::timer::timer()
{
  timer_rb_tree = new rb_tree();
}

xel::timer::~timer()
{
  delete timer_rb_tree;
}

xel::rb_tree::rb_tree()
{
}

xel::rb_tree::~rb_tree()
{
}


void xel::rb_tree::left_rotate(rb_tree_node *nd)
{
  rb_tree_node *temp = nd->right();
  nd->set_right(temp->left());
  if (!is_sentinel(temp->left())){
    temp->left()->set_parent(nd);
  }

  temp->set_parent(nd->parent());
  if(is_root(nd)){
    root = temp;
  } else if (nd == nd->parent()->left()){
    nd->parent()->set_left(temp);
  } else if (nd == nd->parent()->right()){
    nd->parent()->set_right(temp);
  }

  temp->set_left(nd);
  nd->set_parent(temp);

}

void xel::rb_tree::right_rotate(rb_tree_node *nd)
{
  rb_tree_node *temp = nd->left();
  nd->set_left(temp->right());
  if (!is_sentinel(temp->right())){
    temp->right()->set_parent(nd);
  }

  temp->set_parent(nd->parent());
  if(is_root(nd)){
    root = temp;
  } else if (nd == nd->parent()->left()){
    nd->parent()->set_left(temp);
  } else if (nd == nd->parent()->right()){
    nd->parent()->set_right(temp);
  }

  temp->set_right(nd);
  nd->set_parent(temp);
}

void xel::rb_tree::insert_timer_value(rb_tree_node *nd)
{
  rb_tree_node *temp = root;
  rb_tree_node *p;
  for(;;){
    p = nd->key() < temp->key()? temp->left() : temp->right();
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
xel::rb_tree_node *xel::rb_tree::get_min(rb_tree_node *nd)
{
  while(!is_sentinel(nd->left())){
    nd = nd->left();
  }
  return nd;
}
void xel::rb_tree::insert_node(rb_tree_node *nd)
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
  rb_tree_node *temp;

  while(!is_root(nd) && nd->parent()->is_red()){
    if(nd->parent() == nd->parent()->parent()->left()){
      temp = nd->parent()->parent()->right();
      if (temp->is_red()){
        // both parent and parent's slibing is red, just re-color
        nd->parent()->set_color(COLOR::BLACK);
        temp->set_color(COLOR::BLACK);
        nd->parent()->parent()->set_color(COLOR::RED);
        nd = nd->parent()->parent();
      } else {
        // parent is red and parent's slibing is not red
        if (nd == nd->parent()->right()){
          nd = nd->parent();
          left_rotate(nd);
        }
        nd->parent()->set_color(COLOR::BLACK);
        nd->parent()->parent()->set_color(COLOR::RED);
        right_rotate(nd->parent()->parent());
      }

    } else {
      temp = nd->parent()->parent()->left();
      if (temp->is_red()){
        // both parent and parent's slibing is red, just re-color
        nd->parent()->set_color(COLOR::BLACK);
        temp->set_color(COLOR::BLACK);
        nd->parent()->parent()->set_color(COLOR::RED);
        nd = nd->parent()->parent();
      } else {
        // parent is red and parent's slibing is not red
        if (nd == nd->parent()->left()){
          nd = nd->parent();
          right_rotate(nd);
        }
        nd->parent()->set_color(COLOR::BLACK);
        nd->parent()->parent()->set_color(COLOR::RED);
        left_rotate(nd->parent()->parent());
      }
    }
  }
  root->set_color(COLOR::BLACK);
}

void xel::rb_tree::delete_node(rb_tree_node *nd)
{
  rb_tree_node *temp, *sub;

  if(is_sentinel(nd->left())){
    temp = nd->right();
    sub = nd;
  } else if (is_sentinel(nd->right())){
    temp = nd->left();
    sub = nd;
  } else {
    // node is not leaf, get its right children's minimal node
    sub = get_min(nd->right());
    if(is_sentinel(sub->left())){
      temp = sub->left();
    } else {
      temp = sub->right();
    }
  }

  if(is_root(sub)){
    root = temp;
    temp->set_color(COLOR::BLACK);
    return;
  }

  bool b_red = sub->is_red();

  if (sub == sub->parent()->left()){
    sub->parent()->set_left(temp);
  } else {
    sub->parent()->set_right(temp);
  }

  if (sub == nd){
    temp->set_parent(sub->parent());
  } else {
    if (sub->parent() == nd){
      temp->set_parent(sub);
    } else {
      temp->set_parent(sub->parent());
    }

    sub->set_parent(nd->parent());
    sub->set_left(nd->left());
    sub->set_right(nd->right());
    sub->set_color(nd->color());

    if(is_root(nd)){
      root = sub;
    } else {
      if(nd == nd->parent()->left()){
        nd->parent()->set_left(sub);
      } else {
        nd->parent()->set_right(sub);
      }
    }

    if(!is_sentinel(sub->left())){
      sub->left()->set_parent(sub);
    }

    if(!is_sentinel(sub->right())){
      sub->right()->set_parent(sub);
    }
  }
  if(b_red){
    return;
  }

  /*
   * delete fixup
   */
  rb_tree_node *w;

  while(!is_root(temp) && temp->is_black()){
    if(temp == temp->parent()->left()){
      w = temp->parent()->right();
      if(w->is_red()){
        // node's color is black while parent's slibing is red, re-color
        w->set_color(COLOR::BLACK);
        temp->parent()->set_color(COLOR::RED);
        left_rotate(temp->parent());
        w = temp->parent()->right();
      }

      if(w->left()->is_black() && w->right()->is_black()){
        w->set_color(COLOR::RED);
        temp = temp->parent();
      } else {
        if(w->right()->is_black()){
          w->left()->set_color(COLOR::BLACK);
          w->set_color(COLOR::RED);
          right_rotate(w);
          w = temp->parent()->right();
        }
        w->set_color(temp->parent()->color());
        temp->parent()->set_color(COLOR::BLACK);
        w->right()->set_color(COLOR::BLACK);
        left_rotate(temp->parent());
        temp = root;
      }

    } else {
      w = temp->parent()->left();
      if(w->is_red()){
        // node's color is black while parent's slibing is red, re-color
        w->set_color(COLOR::BLACK);
        temp->parent()->set_color(COLOR::RED);
        right_rotate(temp->parent());
        w = temp->parent()->left();
      }

      if(w->left()->is_black() && w->right()->is_black()){
        w->set_color(COLOR::RED);
        temp = temp->parent();
      } else {
        if(w->left()->is_black()){
          w->right()->set_color(COLOR::BLACK);
          w->set_color(COLOR::RED);
          left_rotate(w);
          w = temp->parent()->left();
        }

        w->set_color(temp->parent()->color());
        temp->parent()->set_color(COLOR::BLACK);
        w->left()->set_color(COLOR::BLACK);
        right_rotate(temp->parent());
        temp = root;
      }
    }
  }
  temp->set_color(COLOR::BLACK);
}

