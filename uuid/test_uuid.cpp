#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/functional/hash.hpp>

using namespace std;
using namespace boost::uuids;

int main()
{
	uuid u = nil_uuid();
	boost::hash<uuid> uuid_hasher;
	cout << u << endl;
	cout << uuid_hasher(u) << endl;
	cout << hash_value(u) << endl;

	return 0;
}
