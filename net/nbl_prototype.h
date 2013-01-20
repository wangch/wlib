/*
 * nbl_prototype.h
 *
 * Copyright (c) 2008 by NetBox, Inc.
 *
 * 2008-08-25 wangch
 *
 */

#ifndef NBL_PROTOTYPE_H
#define NBL_PROTOTYPE_H

#include <map>

namespace nbcl
{
/*
class nbl_prototype
{
public:
	virtual ~nbl_prototype() {}
	virtual nbl_prototype* clone() = 0;
};
*/

template<class Key, class Base>
class nbl_creator
{
	std::map<Key, Base*> map_;
public:
	bool add(Key key, Base* value)
	{
		return this->map_.insert(std::pair<Key, Base*>(key, value)).second;
	}
	Base* create(Key key)
	{
		if(this->map_.find(key) != this->map_.end())
			return this->map_[key]->clone();
		return 0;
	}
};

}

#endif

