#include "tests.h"
#include "asserts.h"
#include <vector>
#include <Scenegraph/Group.h>

using namespace std;
using namespace FL;

void GroupTest()
{
	//create group
	ref_ptr<Group> group1(new Group());
	//add nodes to group
	int num1 = 10;
	for (int i = 0; i < num1; ++i)
	{
		Node* node = new Node();
		group1->addChild(node);
	}
	ASSERT_EQ(num1, group1->getNumChildren());
	//duplicate group shallow copy
	ref_ptr<Group> group2(new Group(*group1));
	ASSERT_EQ(num1, group2->getNumChildren());
	//check children
	for (int i = 0; i < num1; ++i)
	{
		Node* node1 = group1->getChild(i);
		Node* node2 = group2->getChild(i);
		ASSERT_EQ(node1, node2);
	}
	//remove children from group1
	int num2 = 5;
	group1->removeChildren(0, num2);
	ASSERT_EQ(num1 - num2, group1->getNumChildren());
	//duplicate group deep copy
	ref_ptr<Group> group3(new Group(*group1, CopyOp::DEEP_COPY_ALL));
	ASSERT_EQ(num1 - num2, group3->getNumChildren());
	//check children
	for (int i = 0; i < num1 - num2; ++i)
	{
		Node* node1 = group1->getChild(i);
		Node* node2 = group3->getChild(i);
		ASSERT_EQ(node1, node2);
	}
	//group of groups
	ref_ptr<Group> group4(new Group());
	group4->addChild(group1.get());
	group4->addChild(group2.get());
	group4->addChild(group3.get());
	//traverse
}