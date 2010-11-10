#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <boost/stm/transaction.hpp>

template <typename _Key, typename _Data>
class list_node : public boost::stm::transaction_object< list_node<_Key, _Data> >
{
public:

	
	list_node() : key_(_Key()), next_(NULL) {
	}

	list_node(const _Key &key, const _Data &data) : key_(key), data_(data), next_(NULL) {
	}

	// zero initialization for native types
	void clear() { key_ = _Key(); next_ = NULL; }

	_Key &key() { return key_; }
	_Key const &key() const { return key_; }

	_Data &data() { return data_; }
	_Data const &data() const { return data_; }

	list_node const *next() const { return next_; }

	void next(list_node const *rhs, boost::stm::transaction &t)
	{
		if (NULL == rhs) next_ = NULL;
		else next_ = &t.find_original(*(list_node<_Key, _Data>*)rhs);
	}

	void next_for_new_mem(list_node const *rhs, boost::stm::transaction &t)
	{
		if (NULL == rhs) next_ = NULL;
		else next_ = &t.find_original(*(list_node<_Key, _Data>*)rhs);
	}

private:

	_Key key_;
	_Data data_;
	list_node *next_;
};

template <typename _Key, typename _Data>
class linked_list
{
	typedef list_node<_Key, _Data> list_node_t;
public:

	linked_list() { head_.key() = _Key(); }

	~linked_list() { quick_clear(); }

	/*
	bool move(list_node<T> const &node1, list_node<T> const &node2)
	{
		using namespace boost::stm;
		bool succeeded1 = true, succeeded2 = true;
		transaction_state state = e_no_state;

		do
		{
			try
			{
				transaction t;
				succeeded1 = internal_remove(node1);
				succeeded2 = internal_insert(node2);
				t.end();
			}
			catch (aborted_transaction_exception&) {}

			if (!succeeded1 || !succeeded2)
			{
				return false; // auto abort of t
			}

		} while (e_committed != state);

		return true;
	}
	*/

	bool insert(list_node_t const &node)
	{
		using namespace boost::stm;

		transaction* tx=current_transaction();
		if (tx!=0) {
			return internal_insert(node, *tx);
		}
		else return false;
	}

	bool insert(const _Key& key, const _Data& data)
	{
		using namespace boost::stm;

		transaction* tx=current_transaction();
		if (tx!=0) {
			return internal_insert(key, data, *tx);
		}
		else return false;
	}

	bool lookup(_Key const &key, _Data& data) const
	{
		using namespace boost::stm;

		transaction* tx=current_transaction();
		if (tx!=0) {
			return internal_lookup(key, data, *tx);
		}
		else return false;
	}

	bool remove(list_node_t const &node)
	{
		using namespace boost::stm;

		transaction* tx=current_transaction();
		if (tx!=0) {
			return internal_remove(node, *tx);
		}
		else return false;
	}

	bool remove(_Key const &key)
	{
		using namespace boost::stm;

		transaction* tx=current_transaction();
		if (tx!=0) {
			return internal_remove(key, *tx);
		}
		else return false;
	}

	void output_list()
	{
		int i = 0;
		for (list_node_t const *cur = head_.next(); cur != NULL; cur = cur->next())
		{
			cout << "element [" << i++ << "]: " << cur->key() << ":" << cur->data() << std::endl;
		}
	}

	int walk_size()
	{
		int i = 0;
		for (list_node_t const *cur = head_.next(); cur != NULL; cur = cur->next())
		{
			++i;
		}

		return i;
	}

	void quick_clear()
	{
		for (list_node_t const *cur = head_.next(); cur != NULL;)
		{
			list_node_t const *prev = cur;
			cur = cur->next();
			delete prev;
		}

		head_.clear();
	}

	boost::stm::transaction_state clear()
	{
		transaction* tx=current_transaction();

		if (tx == NULL)
			return;

		for (list_node_t const *cur = tx->read(head_).next(); cur != NULL;)
		{
			list_node_t const *prev = &tx->read(*cur);
			cur = tx->read(*cur).next();
			tx->delete_memory(*prev);
		}

		tx->write(head_).clear();
	}

private:

	//--------------------------------------------------------------------------
	// find the location to insert the node. if the value already exists, bail
	//--------------------------------------------------------------------------
	bool internal_insert(list_node_t const &rhs, boost::stm::transaction &t)
	{
		list_node_t const *headP = &t.read(head_);

		if (NULL != headP->next())
		{
			list_node_t const *prev = headP;
			list_node_t const *cur = t.read_ptr(headP->next());
			const _Key& key = rhs.key();

			while (true)
			{
				if (cur->key() == key) return false;
				else if (cur->key() > key || !cur->next()) break;

				prev = cur;

				list_node_t const *curNext = t.read_ptr(cur->next());

				if (NULL == curNext) break;

				cur = curNext;
			}

			list_node_t *newNode = t.new_memory_copy(rhs);

			//--------------------------------------------------------------------
			// if cur->next() is null it means our newNode value is greater than
			// cur, so insert ourselves after cur.
			//--------------------------------------------------------------------
			if (key > cur->key()) t.write_ptr((list_node_t*)cur)->next_for_new_mem(newNode, t);
			//--------------------------------------------------------------------
			// otherwise, we are smaller than cur, so insert between prev and cur
			//--------------------------------------------------------------------
			else
			{
				newNode->next(cur, t);
				t.write_ptr((list_node_t*)prev)->next_for_new_mem(newNode, t);
			}
		}
		else
		{
			list_node_t *newNode = t.new_memory_copy(rhs);
			t.write(head_).next_for_new_mem(newNode, t);
		}

		return true;
	}

	bool internal_insert(_Key const& key, _Data const& data, boost::stm::transaction &t)
	{
		//T val = valr;
		list_node_t const *headP = &t.read(head_);

		if (NULL != headP->next())
		{
			list_node_t const *prev = headP;
			list_node_t const *cur = t.read_ptr(headP->next());

			while (true)
			{
				if (cur->key() == key) return false;
				else if (cur->key() > key || !cur->next()) break;

				prev = cur;

				list_node_t const *curNext = t.read_ptr(cur->next());

				if (NULL == curNext) break;

				cur = curNext;
			}
#ifndef BOOST_STM_USES_AS_NEW
			list_node_t node(key, data);
			list_node_t *newNode = t.new_memory_copy(node);
#else
			t.throw_if_forced_to_abort_on_new();
			list_node_t *newNode = t.as_new(new list_node_t(key, data));
#endif
			//--------------------------------------------------------------------
			// if cur->next() is null it means our newNode value is greater than
			// cur, so insert ourselves after cur.
			//--------------------------------------------------------------------
			if (key > cur->key()) t.write_ptr((list_node_t*)cur)->next_for_new_mem(newNode, t);
			//--------------------------------------------------------------------
			// otherwise, we are smaller than cur, so insert between prev and cur
			//--------------------------------------------------------------------
			else
			{
				newNode->next(cur, t);
				t.write_ptr((list_node_t*)prev)->next_for_new_mem(newNode, t);
			}
		}
		else
		{
#ifndef BOOST_STM_USES_AS_NEW
			list_node_t node(key, data);
			list_node_t *newNode = t.new_memory_copy(node);
#else
			t.throw_if_forced_to_abort_on_new();
			list_node_t *newNode = t.as_new(new list_node_t(key, data));
#endif
			t.write(head_).next_for_new_mem(newNode, t);
		}

		return true;
	}
	//--------------------------------------------------------------------------
	// find the location to insert the node. if the value already exists, bail
	//--------------------------------------------------------------------------
	bool internal_lookup(_Key const &key, _Data &data, boost::stm::transaction &t) const
	{
		list_node_t const *cur = &t.read(head_);

		for (; true ; cur = t.read(*cur).next() )
		{
			list_node_t const *trueCur = &t.read(*cur);

			if (trueCur->key() == key)
			{
				data = trueCur->data();
				return true;
			}

			if (NULL == trueCur->next()) break;
		}

		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	bool internal_remove(list_node_t const &rhs, boost::stm::transaction &t)
	{
		list_node_t const *prev = &t.read(head_);

		for (list_node_t const *cur = prev; cur != NULL; prev = cur)
		{
			cur = t.read(*cur).next();

			if (NULL == cur) break;

			if (cur->key() == rhs.key())
			{
				list_node_t const *curNext = t.read(*cur).next();

				t.delete_memory(*cur);
				t.write(*(list_node_t*)prev).next(curNext, t);
				return true;
			}
		}

		return false;
	}

	bool internal_remove(_Key const &key, boost::stm::transaction &t)
	{
		list_node_t const *prev = &t.read(head_);

		for (list_node_t const *cur = prev; cur != NULL; prev = cur)
		{
			cur = t.read(*cur).next();

			if (NULL == cur) break;

			if (cur->key() == key)
			{
				list_node_t const *curNext = t.read(*cur).next();

				t.delete_memory(*cur);
				t.write(*(list_node_t*)prev).next(curNext, t);
				return true;
			}
		}

		return false;
	}
	//--------------------------------------------------------------------------
	// find the location to insert the node. if the value already exists, bail
	//--------------------------------------------------------------------------
	bool internal_insert(list_node_t const &rhs)
	{
		using namespace boost::stm;
		transaction* tx=current_transaction();

		if (tx == NULL)
			return;

		list_node_t const *headP = &tx->read(head_);

		if (NULL != headP->next())
		{
			list_node_t const *prev = headP;
			list_node_t const *cur = tx->read_ptr(headP->next());
			_Key& key = rhs.key();

			while (true)
			{
				if (cur->key() == key) return false;
				else if (cur->key() > key || !cur->next()) break;

				prev = cur;

				list_node_t const *curNext = t.read_ptr(cur->next());

				if (NULL == curNext) break;

				cur = curNext;
			}

			list_node_t *newNode = t.new_memory_copy(rhs);

			//--------------------------------------------------------------------
			// if cur->next() is null it means our newNode value is greater than
			// cur, so insert ourselves after cur.
			//--------------------------------------------------------------------
			if (NULL == cur->next()) t.write_ptr((list_node_t*)cur)->next_for_new_mem(newNode, t);
			//--------------------------------------------------------------------
			// otherwise, we are smaller than cur, so insert between prev and cur
			//--------------------------------------------------------------------
			else
			{
				newNode->next(cur, t);
				t.write_ptr((list_node_t*)prev)->next_for_new_mem(newNode, t);
			}
		}
		else
		{
			list_node_t *newNode = t.new_memory_copy(rhs);
			t.write(head_).next_for_new_mem(newNode, t);
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	bool internal_remove(list_node_t const &rhs)
	{
		using namespace boost::stm;
		transaction *tx = current_transaction();

		if (tx == NULL) return false;

		list_node_t const *prev = &tx->read(head_);

		for (list_node_t const *cur = prev; cur != NULL;
			prev = cur, cur = tx->read(*cur).next())
		{
			if (cur->key() == rhs.key())
			{
				tx->write(*(list_node_t*)prev).next(tx->read_ptr(cur)->next(), t);
				tx->delete_memory(*cur);
				return true;
			}
		}

		return false;
	}

	list_node_t head_;
};

#endif // LINKED_LIST_H
