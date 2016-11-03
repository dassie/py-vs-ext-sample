	#include <boost/python.hpp>
	#include <string>

	std::string sayhello ()
	{
		return std::string("Hello");
	}

	BOOST_PYTHON_MODULE(MODULE_NAME)
	{
		using namespace boost::python;
		def("sayhello", sayhello);
	}