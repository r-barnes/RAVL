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

    //Add a new node to the tree, but perform garbage collection such that the
    //spaces occupied by removed nodes are reused
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

    //Return the right child of the node
    node<T> & rc(int root) { return nodes[nodes[root].rightChild]; }

    //Return the left child of the node
    node<T> & lc(int root) { return nodes[nodes[root].leftChild]; }

    //Does the node have a right child?
    bool has_right(int root) const { return root!=-1 && !nodes[nodes[root].rightChild].empty; }

    //Does the node have a left child?
    bool has_left(int root) const { return root!=-1 && !nodes[nodes[root].leftChild ].empty; }

    void print_dot_helper(int root, std::ostream& out) const {
      if(!nodes[root].empty){
        out<<root<<"->"<<nodes[root].leftChild<<std::endl;
        print_dot_helper(nodes[root].leftChild, out);
        out<<root<<"->"<<nodes[root].rightChild<<std::endl;
        print_dot_helper(nodes[root].rightChild, out);
      }
    }

    //Calculates the balance factor of a node from its children
    int bf(int root) const {
      return nodes[nodes[root].leftChild].height-nodes[nodes[root].rightChild].height;
    }

    //Calculates the height of a node from its children
    void calc_height(int node){
      nodes[node].height=1+std::max( lc(node).height, rc(node).height );
    }

    //Used to perform rotations
    void replace_child(int root, int oldChild, int newChild){
      if(root==-1)
        return;
      else if(nodes[root].leftChild==oldChild)
        nodes[root].leftChild=newChild;
      else if(nodes[root].rightChild==oldChild)
        nodes[root].rightChild=newChild;
    }

    //Rotation
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

    //Rotation
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

    //Rotation
    void rotate_parents(int root, int pivot){
      replace_child(nodes[root].parent, root, pivot);
      nodes[pivot].parent=nodes[root].parent;
      nodes[root].parent=pivot;
      if(nodes[pivot].parent==-1) treeroot=pivot;
    }

    //Rebalancing to be performed at each node by the rebalancing loop
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

    //Rebalancing loop to be performed following insertion of a node
    void insert_rebalance(int root){
      while(root!=-1){
        calc_height(root);    //TODO: Bailing early if the balance factor has not changed would save time
        root=insert_balances(root);
        root=nodes[root].parent;
      }
    }

    //Rebalancing to be performed following the removal of a node
    void remove_rebalance(int root){
      while(root!=-1){
        calc_height(root);    //TODO: Bailing early would save time
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

    //Given a datum, find where it should or would be located in the tree
    int find_place_for_datum(T datum) const {
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

    //Find the "successor node", the next highest node in the sequence
    int successor(int root) const {
      root=nodes[root].rightChild;
      while(!nodes[root].empty)
        root=nodes[root].leftChild;
      return nodes[root].parent;
    }

    //Common constructor used by all the others
    void init(){
      nodes.push_back(node<T>());
      treeroot=0;
      nodecount=0;
      rotation_count=0;
    }

    ///Remove a node, given its root id
    void removeNode(int root){
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

    ///Returns the root of the tree
    T& top() { return nodes[treeroot].data; }

    ///Returns the root of the tree, and removes it
    T pop() {
      T temp=nodes[treeroot].data;
      removeNode(treeroot);
      return temp;
    }

    ///Inserts a new datum into the appropriate place in the tree
    void insert(T datum){
      int root=find_place_for_datum(datum);
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

    ///Given a pointer to an array of datums, insert the whole array
    void insert(const T *datums, int n){
      for(int i=0;i<n;i++)
        insert(datums[i]);
    }

    ///Given a datum, find it in the tree and remove it and its node
    void remove(T datum){
      #if AVLDEBUG
      std::cerr<<"You want to remove "<<datum<<std::endl;
      #endif
      int root=find_place_for_datum(datum);
      if(nodes[root].empty)
        return;
      nodecount--;
      removeNode(root);
    }

    ////////////////////////
    //Output diagrams
    ////////////////////////

    /**
      @brief Prints the tree in order of its levels.

          This prints first the root of the tree, then the two nodes of the
          root, then the nodes of the nodes, and so on. It is a breadht-first
          exploration of the tree.
    */
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

    ///Prints a dot (graphviz) diagram of the tree.
    void print_dot(std::ostream &out) const {
      static int digraphlabel=0;
      digraphlabel++;
      out<<"digraph "<< digraphlabel <<" {"<<std::endl;
      print_dot_helper(treeroot, out);
      for(int i=0;i<nodes.size();i++)
        if(!nodes[i].empty)
          out<<i<<" [label=\""<<nodes[i].data<<"\"]"<<std::endl;
        else
          out<<i<<" [style=invis]"<<std::endl;
      out<<"}"<<std::endl;
    }

    ///Prints output of the tree suitable for use with the LaTeX qTree package
    void qtreePrint(std::ostream& out, int root=-1) const {
      if(root==-1) root=treeroot;

      if(nodes[root].empty){
        out<<" \\missing ]";
        return;
      }

      out<<" [."<<nodes[root].data;
      qtreePrint(out, nodes[root].leftChild);
      out<<"] ";
      qtreePrint(out, nodes[root].rightChild);
      out<<"]";
    }
};
