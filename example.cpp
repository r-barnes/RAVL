#include "ravl.hpp"
using namespace std;

int main(){
  //Define some data
	int data[]={21,13,29,8,18,26,32,5,11,16};
  int data2[]={55,56,57};

  //Define a new AVL tree
	AvlTree<int> tree;

  //Insert data with loop
	for(int i=0;i<10;i++)
		tree.insert(data[i]);

  //Insert data with built-in loop
  tree.insert(data2,3);

  //Print a representation of the tree
	tree.qtreePrint(cout);

  cout<<endl<<endl;

  //Remove some nodes
  tree.remove(13);
  tree.remove(26);

  //Print another representation of the tree
	tree.qtreePrint(cout);

  //Print the number of nodes in the tree
	cout<<endl<<endl<<"Size: "<<tree.size()<<endl;

  cout<<"Value of top node: "<<tree.top()<<endl;
  tree.pop();
  cout<<"Value of top node: "<<tree.top()<<endl;

	return 0;
}
