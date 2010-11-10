#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <boost/functional/hash.hpp>
#include <boost/stm/transaction.hpp>
#include "linked_list.h"

int const kBuckets = 256;

template <typename _Key, typename _Data>
class hash_map
{
	typedef list_node<_Key, _Data> list_node_t;
public:

	bool lookup(const _Key &key, _Data &data) const
	{
		return buckets_[hasher_(key) % kBuckets].lookup(key, data);
	}

	bool insert(const _Key &key, const _Data &data)
	{
		return buckets_[hasher_(key) % kBuckets].insert(key, data);
	}

	bool remove(const _Key &key)
	{
		return buckets_[hasher_(key) % kBuckets].remove(key);
	}

	size_t walk_size()
	{
		size_t count = 0;
		for (int i = 0; i < kBuckets; ++i)
		{
			count += buckets_[i].walk_size();
		}

		return count;
	}

	void output_map()
	{
		for (int i = 0; i < kBuckets; ++i)
		{
			buckets_[i].output_list();
		}
	}

private:
	linked_list<_Key, _Data> buckets_[kBuckets];
	boost::hash<_Key> hasher_;
};

#endif // HASH_MAP_H
