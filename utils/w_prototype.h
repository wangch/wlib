/*
 * w_prototype.h
 */

#ifndef W_PROTOTYPE_H_
#define W_PROTOTYPE_H_

#include <map>

namespace wlib {
/*
class w_prototype
{
public:
	virtual ~w_prototype() {}
	virtual w_prototype* clone() = 0;
};
*/

template<class Key, class Base>
class w_creator {
	std::map<Key, Base*> map_;
public:
	bool add(Key key, Base* value) {
		return this->map_.insert(std::pair<Key, Base*>(key, value)).second;
	}

	Base* create(Key key) {
		if(this->map_.find(key) != this->map_.end()) {
			return this->map_[key]->clone();
      }
		return 0;
	}
};

}

#endif // W_PROTOTYPE_H_

