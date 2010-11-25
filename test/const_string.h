#ifndef CONST_STRING_H
#define CONST_STRING_H
	
class const_string {
public:
	// Constructors
	const_string()
	{
	}
	
	const_string( std::string const& s )
	{
	}
	
	const_string( char const* s )
	{
	}
	
	const_string( char const* s, size_t length )
	{
	}
	
	const_string( char const* begin, char const* end )
	{
	}
	
	// Access methods
	char const* data() const;
	size_t      length() const;
	bool        is_empty() const;
	
	};

#endif // CONST_STRING_H

