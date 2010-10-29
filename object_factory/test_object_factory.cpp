#include <iostream>
#include <map>
#include <exception>
#include <boost/functional/factory.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace std;

class object_base
{
public:
	object_base() : m_base(0), m_line(0)
	{
		memset(m_file, 0, sizeof(m_file));
		cout << "object_base()" << endl;
	}

	virtual ~object_base()
	{
		if (m_line)
			cout << "~object_base() at " << m_file << ":" << m_line << endl;
		else
			cout << "~object_base()" << endl;
	}

	static void* operator new(size_t size) {
		cout << "object_base::new " << size << endl;
		return malloc(size);
	}

	static void operator delete(void* deletable, size_t size) {
		cout << "object_base::delete " << size << endl;
		free(deletable);
	}

	void set_trace(const char *file, int line)
	{
		strcpy(m_file, file);
		m_line = line;
	}

	virtual void print()
	{
		cout << "base" << endl;
	}

private:
	int m_base;
	char m_file[50];
	int m_line;
};

class object_a : public object_base
{
public:
	object_a() : m_a(0)
	{
		cout << "object_a()" << endl;
	}

	virtual ~object_a()
	{
		cout << "~object_a()" << endl;
	}

	void print_a()
	{
		cout << "a" << endl;
	}

	virtual void print()
	{
		print_a();
	}

private:
	int m_a;
};

class object_b : public object_base
{
public:
	object_b() : m_b(0)
	{
		cout << "object_b()" << endl;
	}

	virtual ~object_b()
	{
		cout << "~object_b()" << endl;
	}

	void print_b()
	{
		cout << "b" << endl;
	}

	virtual void print()
	{
		print_b();
	}

private:
	int m_b;
};

class object_factory
{
	typedef boost::function< boost::shared_ptr<object_base>() > a_factory;
public:
	object_factory()
	{
		m_factories["a"] = boost::factory<boost::shared_ptr<object_a> >();
		m_factories["b"] = boost::factory<boost::shared_ptr<object_b> >();
	}

	virtual ~object_factory()
	{
	}

	boost::shared_ptr<object_base> create(const std::string& classname, const char* file = NULL, int line = 0)
	{
		cout << "Create " << classname << " at " << file << ":" << line << endl;
		boost::shared_ptr<object_base> ob = m_factories[classname]();
		ob->set_trace(file, line);
		return ob;
	}
	

private:
	std::map<std::string,a_factory> m_factories;

};

int main()
{
	try
	{
	cout << "*************************** Test shared_ptr" << endl;
	{
		// direct use shared_ptr
		boost::shared_ptr<object_base> obase(new object_base());
		boost::shared_ptr<object_a> oa(new object_a());
		boost::shared_ptr<object_b> ob(new object_b());

		boost::shared_ptr<object_base> oa_base = boost::dynamic_pointer_cast<object_base, object_a>(oa);
		oa_base->print();
	}

	cout << "*************************** Test factory" << endl;
	{
		object_a *oa = boost::factory<object_a*>()();
		object_b *ob = boost::factory<object_b*>()();

		oa->print();
		ob->print();

		delete oa;
		delete ob;
	}

	cout << "*************************** Test shared_ptr and factory" << endl;
	{
		// use factory and shared_ptr
		boost::shared_ptr<object_a> oa = boost::factory<boost::shared_ptr<object_a> >()();
		boost::shared_ptr<object_b> ob = boost::factory<boost::shared_ptr<object_b> >()();

		oa->print();
		ob->print();	
	}

	cout << "*************************** Test all" << endl;
	{
		object_factory of;
#define CREATE_OBJ(X) of.create(X, __FILE__, __LINE__)

		boost::shared_ptr<object_base> oa = CREATE_OBJ("a");
		boost::shared_ptr<object_base> ob = CREATE_OBJ("b");
		oa->print();
		ob->print();
	}
	}
	catch(exception& e)
	{
		cout << e.what() << endl;
	}

	return 0;
}

