#include <iostream>
#include <string>
#include <boost/regex.hpp>

using namespace std;

int main( ) {

   std::string s, sre;
   boost::regex re;
   boost::cmatch matches;

   while(true)
   {
      cout << "Expression: ";
      cin >> sre;
      if (sre == "quit")
      {
         break;
      }

	cout << "Expression: " << sre << endl;
      cout << "String:     ";
      cin >> s;
	cout << "String: " << s << endl;

      try
      {
         // Assignment and construction initialize the FSM used
         // for regexp parsing
         re = sre;
      }
      catch (boost::regex_error& e)
      {
         cout << sre << " is not a valid regular expression: \""
              << e.what() << "\"" << endl;
         continue;
      }
      // if (boost::regex_match(s.begin(), s.end(), re))
      if (boost::regex_match(s.c_str(), matches, re))
      {
         // matches[0] contains the original string.  matches[n]
         // contains a sub_match object for each matching
         // subexpression
         for (int i = 1; i < matches.size(); i++)
         {
            // sub_match::first and sub_match::second are iterators that
            // refer to the first and one past the last chars of the
            // matching subexpression
            string match(matches[i].first, matches[i].second);
            cout << "\tmatches[" << i << "] = " << match << endl;
         }
      }
      else
      {
         cout << "The regexp \"" << re << "\" does not match \"" << s << "\"" << endl;
      }
   }
}
