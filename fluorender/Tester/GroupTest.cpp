#include "tests.h"
#include "asserts.h"
#include "VisitorTest.h"
#include <vector>
#include <Scenegraph/Group.h>
#include <Scenegraph/ValueUpdateVisitor.h>

using namespace std;
using namespace FL;

void GroupTest()
{
	//create group
	//ref_ptr<Group> group1(new Group());
	Group* group1 = new Group();
	group1->setName("group1");
	//add nodes to group
	int num1 = 10;
	for (int i = 0; i < num1; ++i)
	{
		Node* node = new Node();
		group1->addChild(node);
	}
	ASSERT_EQ(num1, group1->getNumChildren());
	//duplicate group shallow copy
	//ref_ptr<Group> group2(new Group(*group1));
	Group* group2 = new Group(*group1);
	group2->setName("group2");
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
	//ref_ptr<Group> group3(new Group(*group1, CopyOp::DEEP_COPY_ALL));
	Group* group3 = new Group(*group1, CopyOp::DEEP_COPY_ALL);
	group3->setName("group3");
	ASSERT_EQ(num1 - num2, group3->getNumChildren());
	//check children
	for (int i = 0; i < num1 - num2; ++i)
	{
		Node* node1 = group1->getChild(i);
		Node* node2 = group3->getChild(i);
		ASSERT_EQ(node1, node2);
	}
	//group of groups
	//ref_ptr<Group> group4(new Group());
	Group* group4 = new Group();
	group4->setName("group4");

	group4->addChild(group1);
	group4->addChild(group2);
	group4->addChild(group3);
	group4->addChild(group3);
	group3->addChild(group4);

	std::vector<ref_ptr<Node>> list;
	list.push_back(group1);
	list.push_back(group2);
	list.push_back(group3);
	list.push_back(group4);
	//traverse
	InfoVisitor visitor;
	group4->accept(visitor);
}

void GroupTest2()
{
	//create group
	Group* group1 = new Group();
	group1->setName("group1");
	group1->addValue("value1", 0.0);
	group1->addValue("value2", 0.0);
	group1->addValue("value3", 0.0);

	//add nodes to group
	int num1 = 3;
	for (int i = 0; i < num1; ++i)
	{
		Node* node = new Node();
		node->addValue("value1", 0.0);
		node->addValue("value2", 0.0);
		node->addValue("value3", 0.0);
		group1->addChild(node);
	}
	ASSERT_EQ(num1, group1->getNumChildren());

	//duplicate group shallow copy
	Group* group2 = new Group(*group1);
	group2->setName("group2");
	ASSERT_EQ(num1, group2->getNumChildren());

	//check children
	for (int i = 0; i < num1; ++i)
	{
		Node* node1 = group1->getChild(i);
		Node* node2 = group2->getChild(i);
		ASSERT_EQ(node1, node2);
	}

	//duplicate group deep copy
	Group* group3 = new Group(*group1, CopyOp::DEEP_COPY_ALL);
	group3->setName("group3");
	ASSERT_EQ(num1, group3->getNumChildren());

	//group of groups
	Group* group4 = new Group();
	group4->setName("group4");
	group4->addValue("value1", 0.0);
	group4->addValue("value2", 0.0);
	group4->addValue("value3", 0.0);

	group4->addChild(group1);
	group4->addChild(group2);
	group4->addChild(group3);
	group4->addChild(group3);

	std::vector<ref_ptr<Node>> list;
	list.push_back(group1);
	list.push_back(group2);
	list.push_back(group3);
	list.push_back(group4);

	//traverse
	InfoVisitor visitor;
	group4->accept(visitor);

	//sync value1
	ValueUpdateVisitor update;
	update.setType(SYNC_VALUE);
	update.setValueName("value1");
	group4->accept(update);
	group1->getChild(0)->setValue("value1", 1.0);
	//traverse
	group4->accept(visitor);

	//sync all
	update.setType(SYNC_ALL_VALUES);
	group4->accept(update);
	group2->getChild(1)->setValue("value2", 2.0);
	//traverse
	group4->accept(visitor);
	group3->getChild(2)->setValue("value3", 3.0);
	//traverse
	group4->accept(visitor);

	//unsync
	//update.setType(UNSYNC_VALUE);
	//group4->accept(update);
}