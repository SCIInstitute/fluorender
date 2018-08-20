#pragma once
#include <Scenegraph/Group.h>
#include <Scenegraph/VolumeData.h>
#include <Scenegraph/Annotations.h>
#include <Flobject/Value.h>
#include <iostream>
#include <string>

using namespace std;
using namespace FL;

class VolumeDataTest : public VolumeData
{
public:
	virtual void objectChanged(void* ptr, const std::string &exp)
	{
		Referenced* refd = static_cast<Referenced*>(ptr);
		if (refd->className() == std::string("Value"))
		{
			Value* value = dynamic_cast<Value*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				"Value:" << value->getName() << " has changed." << std::endl;
		}
		else if (refd->className() == std::string("Annotations"))
		{
			Annotations* ann = dynamic_cast<Annotations*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				ann->className() << ":" << ann->getName() << " has changed." << std::endl;
		}
		else if (refd->className() == std::string("VolumeData"))
		{
			VolumeData* vd = dynamic_cast<VolumeData*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				vd->className() << ":" << vd->getName() <<
				":" << exp << " has changed." << std::endl;
		}
	}
};

class AnnotationsTest : public Annotations
{
public:
	virtual void objectChanged(void* ptr, const std::string &exp)
	{
		Referenced* refd = static_cast<Referenced*>(ptr);
		if (refd->className() == std::string("Value"))
		{
			Value* value = dynamic_cast<Value*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				"Value:" << value->getName() << " has changed." << std::endl;
		}
		else if (refd->className() == std::string("Annotations"))
		{
			Annotations* ann = dynamic_cast<Annotations*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				ann->className() << ":" << ann->getName() << " has changed." << std::endl;
		}
		else if (refd->className() == std::string("VolumeData"))
		{
			VolumeData* vd = dynamic_cast<VolumeData*>(refd);
			std::cout << this->className() << ":" << this->getName() <<
				" was notified that " <<
				vd->className() << ":" << vd->getName() <<
				":" << exp << " has changed." << std::endl;
		}
	}
};