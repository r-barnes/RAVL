#include<vector>
#include<queue>
#include<fstream>
#include<iostream>

//#define AVLDEBUG 1

template <class T>
class node {
  public:
    int leftChild, rightChild, parent;
    bool empty;
    T data;
    int height;
    node() : leftChild(-1), rightChild(-1), parent(-1), empty(true), height(0) {}
    void set_data(T &datum){ data=datum;empty=false; }
};

template <class T>
class AvlTree {
  private:
    std::vector< node<T> > nodes;
    std::queue<int> free_spaces;
    int treeroot;
    int nodecount;

    int rotation_count;

    bool a_less_b(const T &a, const node<T> &b)  const { return a<b.data; }
    bool a_equal_b(const T &a, const node<T> &b) const { return a==b.data;}

    int newnode(int parent){
      int index;
      if(!free_spaces.empty()){
        index=free_spaces.front();
        free_spaces.pop();
        nodes[index]=node<T>();
      } else {
        index=nodes.size();
        nodes.push_back(node<T>());
      }
      nodes[index].parent=parent;
      return index;
    }

    node<T> & rc(int root) { return nodes[nodes[root].rightChild]; }
    node<T> & lc(int root) { return nodes[nodes[root].leftChild]; }
    bool has_right(int root) const { return root!=-1 && !nodes[nodes[root].rightChild].empty; }
    bool has_left(int root) const { return root!=-1 && !nodes[nodes[root].leftChild ].empty; }

    void print_dot_helper(int root, std::ostream& out) const {
      if(!nodes[root].empty){
        out<<root<<"->"<<nodes[root].leftChild<<std::endl;
        print_dot_helper(nodes[root].leftChild, out);
        out<<root<<"->"<<nodes[root].rightChild<<std::endl;
        print_dot_helper(nodes[root].rightChild, out);
      }
    }

    int bf(int root) const {
      return nodes[nodes[root].leftChild].height-nodes[nodes[root].rightChild].height;
    }

    void calc_height(int node){
      nodes[node].height=1+std::max( lc(node).height, rc(node).height );
    }

    void replace_child(int root, int oldChild, int newChild){
      if(root==-1)
        return;
      else if(nodes[root].leftChild==oldChild)
        nodes[root].leftChild=newChild;
      else if(nodes[root].rightChild==oldChild)
        nodes[root].rightChild=newChild;
    }

    int rotate_left(int root){
      #if AVLDEBUG
      std::cerr<<"Rotating "<<nodes[root].data<<" left."<<std::endl;
      #endif
      int pivot=nodes[root].rightChild;
      nodes[root].rightChild=nodes[pivot].leftChild;
      nodes[nodes[pivot].leftChild].parent=root;
      nodes[pivot].leftChild=root;
      rotate_parents(root,pivot);
      calc_height(root);
      calc_height(pivot);
      return pivot;
    }

    int rotate_right(int root){
      #if AVLDEBUG
      std::cerr<<"Rotating "<<nodes[root].data<<" right."<<std::endl;
      #endif
      int pivot=nodes[root].leftChild;
      nodes[root].leftChild=nodes[pivot].rightChild;
      nodes[nodes[pivot].rightChild].parent=root;
      nodes[pivot].rightChild=root;
      rotate_parents(root,pivot);
      calc_height(root);
      calc_height(pivot);
      return pivot;
    }

    void rotate_parents(int root, int pivot){
      replace_child(nodes[root].parent, root, pivot);
      nodes[pivot].parent=nodes[root].parent;
      nodes[root].parent=pivot;
      if(nodes[pivot].parent==-1) treeroot=pivot;
    }

    int insert_balances(int root){
      if(bf(root)==-2){
        if(bf(nodes[root].rightChild)==-1){        //Single left
          #if AVLDEBUG
          std::cerr<<"Calling for single left rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          root=rotate_left(root);
        } else if(bf(nodes[root].rightChild)==+1){  //Right-Left
          #if AVLDEBUG
          std::cerr<<"Calling for right-left rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          rotate_right(nodes[root].rightChild);
          root=rotate_left(root);
        }
      } else if(bf(root)==2){
        if(bf(nodes[root].leftChild)==+1){        //Single right
          #if AVLDEBUG
          std::cerr<<"Calling for single right rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          root=rotate_right(root);
        }  else if(bf(nodes[root].leftChild)==-1){  //Left-right
          #if AVLDEBUG
          std::cerr<<"Calling for left-right rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          rotate_left(nodes[root].leftChild);
          root=rotate_right(root);
        }
      }
      return root;
    }

    void insert_rebalance(int root){
      while(root!=-1){
        calc_height(root);    //TODO: Bail early if the balance factor has not changed
        root=insert_balances(root);
        root=nodes[root].parent;
      }
    }

    void remove_rebalance(int root){
      while(root!=-1){
        calc_height(root);    //TODO: Bail early somewhere in here
        if(bf(root)==2 && bf(nodes[root].leftChild)==0){
          #if AVLDEBUG
          std::cerr<<"Calling for rb single right rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          root=rotate_right(root);
        }  else if(bf(root)==-2 && bf(nodes[root].rightChild)==0){
          #if AVLDEBUG
          std::cerr<<"Calling for rb single left rotation on "<<nodes[root].data<<"."<<std::endl;
          #endif
          rotation_count++;
          root=rotate_left(root);
        }  else
          root=insert_balances(root);
        root=nodes[root].parent;
      }    
    }

    int searcher(T datum) const {
      int root=treeroot;
      while(!nodes[root].empty)
        if(a_equal_b(datum,nodes[root]))
          return root;
        else if(a_less_b(datum,nodes[root]))
          root=nodes[root].leftChild;
        else
          root=nodes[root].rightChild;
      return root;
    }

    int successor(int root) const {
      root=nodes[root].rightChild;
      while(!nodes[root].empty)
        root=nodes[root].leftChild;
      return nodes[root].parent;
    }

    void init(){
      nodes.push_back(node<T>());
      treeroot=0;
      nodecount=0;
      rotation_count=0;
    }

  public:
    AvlTree(){
      init();
    }

    AvlTree(const T *datums, int n){
      init();
      insert(datums,n);
    }

    ///Return the number of rotations that have been made in the tree
    int rotations() const { return rotation_count; }

    ///Reset the number of rotations that have been made in the tree
    void reset_rotations() { rotation_count=0; }

    ///Return the number of nodes in the tree
    int size() const { return nodecount; }

    ///Prints a dot (graphviz) diagram of the tree.
    void print_dot(std::ostream &out, bool diagnostic=false) const {
      static int digraphlabel=0;
      digraphlabel++;
      out<<"digraph "<< digraphlabel <<" {"<<std::endl;
      print_dot_helper(treeroot, out);
      for(int i=0;i<nodes.size();i++)
        if(diagnostic){
          out<<i<<" [label=\"i"<<i<<":h"<<nodes[i].height<<":p"<<nodes[i].parent;
          if(!nodes[i].empty)
            out<<":d"<<nodes[i].data<<"\"]"<<std::endl;
          else
            out<<"\",shape=polgyon]"<<std::endl;
        } else
          if(!nodes[i].empty)
            out<<i<<" [label=\""<<nodes[i].data<<"\"]"<<std::endl;
          else
            out<<i<<" [style=invis]"<<std::endl;
      out<<"}"<<std::endl;
    }

    void insert(T datum){
      int root=searcher(datum);
      if(!nodes[root].empty)
        return;

      nodecount++;

      nodes[root].set_data(datum);

      int temp=newnode(root);
      nodes[root].leftChild=temp;
      temp=newnode(root);
      nodes[root].rightChild=temp;

      insert_rebalance(root);
    }

    void insert(const T *datums, int n){
      for(int i=0;i<n;i++)
        insert(datums[i]);
    }

    void nodeInfo(int root) const {
      std::cerr<<"id="<<root<<", data="<<nodes[root].data<<", lc="<<nodes[root].leftChild<<", rc="<<nodes[root].rightChild<<", p="<<nodes[root].parent<<std::endl;
    }

    void removeNode(int root){
//      nodeInfo(root);
      int parent=nodes[root].parent;
      if(!has_left(root) && !has_right(root)){
        //I am a leaf, so free me and my children
        #if AVLDEBUG
        std::cerr<<"Removing leaf "<<nodes[root].data<<std::endl;
        #endif
        free_spaces.push(nodes[root].rightChild);
        free_spaces.push(nodes[root].leftChild);
        nodes[root]=node<T>();
        nodes[root].parent=parent;
        root=parent;
      } else if(has_left(root) && !has_right(root)){
        #if AVLDEBUG
        std::cerr<<"Removing single child node "<<nodes[root].data<<std::endl;
        #endif
        replace_child(parent,root,nodes[root].leftChild);
        nodes[nodes[root].leftChild].parent=parent;
        if(root==treeroot)
          treeroot=nodes[root].leftChild;
        nodes[root]=node<T>();
        free_spaces.push(root);
      } else if(has_right(root) && !has_left(root)){
        #if AVLDEBUG
        std::cerr<<"Removing single child node "<<nodes[root].data<<std::endl;
        #endif
        replace_child(parent,root,nodes[root].rightChild);
        nodes[nodes[root].rightChild].parent=parent;
        if(root==treeroot)
          treeroot=nodes[root].rightChild;
        nodes[root]=node<T>();
        free_spaces.push(root);
      } else {                    //D3
        #if AVLDEBUG
        std::cerr<<"Finding successor for "<<nodes[root].data<<std::endl;
        #endif
        int snode=successor(root);
        #if AVLDEBUG
        std::cerr<<"\tSuccessor was "<<nodes[snode].data<<std::endl;
        #endif
        nodes[root].data=nodes[snode].data;
        parent=nodes[snode].parent;
        removeNode(snode);
        root=snode;
      }
      remove_rebalance(parent);
    }

    void remove(T datum){
      #if AVLDEBUG
      std::cerr<<"You want to remove "<<datum<<std::endl;
      #endif
      int root=searcher(datum);
      if(nodes[root].empty)
        return;
      nodecount--;
      removeNode(root);
    }

    void levelPrint(std::ostream& out) const {
      std::queue<int> bfs;
      bfs.push(treeroot);
      while(!bfs.empty()){
        int root=bfs.front();bfs.pop();
        if(nodes[root].empty) continue;
        out<<nodes[root].data<<",";
        bfs.push(nodes[root].leftChild);
        bfs.push(nodes[root].rightChild);
      }
      out<<std::endl;
    }

    void treePrint(std::ostream& out, int root=-1) const {
      if(root==-1) root=treeroot;

      if(nodes[root].empty){
        out<<" \\missing ]";
        return;
      }

      out<<" [."<<nodes[root].data;
      treePrint(out, nodes[root].leftChild);
      out<<"] ";
      treePrint(out, nodes[root].rightChild);
      out<<"]";
    }
};
