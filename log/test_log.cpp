#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/log/common.hpp>
#include <boost/log/filters.hpp>
#include <boost/log/formatters.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/utility/init/to_console.hpp>
#include <boost/log/utility/init/common_attributes.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace fmt = boost::log::formatters;
namespace keywords = boost::log::keywords;

using namespace boost;


enum severity_level
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

BOOST_LOG_DECLARE_GLOBAL_LOGGER(my_logger, boost::log::sources::severity_logger_mt<severity_level>)

// The formatting logic for the severity level
template< typename CharT, typename TraitsT > 
inline std::basic_ostream< CharT, TraitsT >& operator<< (std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
	static const char* const str[] =
	{
		"trace",
		"debug",
		"info",
		"warning",
		"error",
		"fatal"
	};
	if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
		strm << str[lvl];
	else
		strm << static_cast< int >(lvl);
	return strm;
}

void init_log(const char* filename)
{
	try
	{
		logging::core::get()->add_global_attribute("Scope", boost::make_shared< attrs::named_scope >());

	// Open a rotating text file
		shared_ptr< std::ostream > strm(new std::ofstream(filename));
		if (!strm->good())
			throw std::runtime_error("Failed to open log file");

	// Create a text file sink
		shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > sink(
				new sinks::synchronous_sink< sinks::text_ostream_backend >);

		sink->locked_backend()->add_stream(strm);

		sink->locked_backend()->set_formatter(
				fmt::format("[%1%][%2%][%3%] - %4%")
				% fmt::date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
				% fmt::attr< severity_level >("Severity", std::nothrow)
				% fmt::attr< boost::thread::id >("ThreadID")
				% fmt::message()
						     );

		logging::init_log_to_console(std::clog, keywords::format = "%TimeStamp% : %_%");

	// Add it to the core
		logging::core::get()->add_sink(sink);

		// Also let's add some commonly used attributes, like timestamp and record counter.
		logging::add_common_attributes();

	}
	catch (std::exception& e)
	{
		std::cout << "FAILURE: " << e.what() << std::endl;
	}
}

class dump_data
{
	const unsigned char* data_;
	const int len_;
public:
	explicit dump_data(const unsigned char* data, const int len) : data_(data), len_(len) {}

	template< typename CharT, typename TraitsT >
	friend std::basic_ostream< CharT, TraitsT >& operator<< (std::basic_ostream< CharT, TraitsT >& strm, dump_data const& obj)
	{
		strm << std::endl << std::hex;
		for (int i = 0; i < obj.len_; i++)
		{
			strm << std::setw(2) << std::setfill('0') << (unsigned int)obj.data_[i] << " ";
			if ((i+1) % 16 == 0) strm << std::endl; 
		}
		return strm;
	}
};

int main(int, char*[])
{
	init_log("test.log");
	BOOST_LOG_SEV(my_logger::get(), trace) << "A trace severity message";
	char a[256] = {0};
	for (int i = 0; i < 256; i++) *(unsigned char*)&a[i] = i;
	BOOST_LOG_SEV(my_logger::get(), trace) << dump_data((unsigned char*)a, sizeof(a));
	std::cout.printf("%d\n", a[0]);
}
