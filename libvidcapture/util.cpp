#include "util.h"

#include <string>
#include <vector>
#include <locale>
#include <functional>
#include <iostream>
//
//template<class E,
//class T = std::char_traits<E>,
//class A = std::allocator<E> >
//
//class Widen : public std::unary_function<
//	const std::string&, std::basic_string<E, T, A> >
//{ 
//	std::locale loc_;
//	const std::ctype<E>* pCType_;
//
//	// No copy-constructor, no assignment operator...
//	Widen(const Widen&);
//	Widen& operator= (const Widen&);
//
//public: 
//	// Constructor...
//	Widen(const std::locale& loc = std::locale()) : loc_(loc)
//	{
//#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6.0...
//		using namespace std; 
//		pCType_ = &_USE(loc, ctype<E> );
//#else 
//		pCType_ = &std::use_facet<std::ctype<E> >(loc);
//#endif 
//	} 
//
//	// Conversion... 
//	std::basic_string<E, T, A> operator() (const std::string& str) const
//	{ 
//		typename std::basic_string<E, T, A>::size_type srcLen =
//			str.length();
//		const char* pSrcBeg = str.c_str();
//		std::vector<E> tmp(srcLen);
//
//		pCType_->widen(pSrcBeg, pSrcBeg + srcLen, &tmp[0]);
//		return std::basic_string<E, T, A>(&tmp[0], srcLen);
//	}
//};
//
//
//template<class E,
//class T = std::char_traits<E>,
//class A = std::allocator<E> >
//
//class Narrow : public std::unary_function<
//	const std::string&, std::basic_string<E, T, A> >
//{ 
//	std::locale loc_;
//	const std::ctype<E>* pCType_;
//
//	// No copy-constructor, no assignment operator...
//	Narrow(const Narrow&);
//	Narrow& operator= (const Narrow&);
//
//public: 
//	// Constructor...
//	Narrow(const std::locale& loc = std::locale()) : loc_(loc)
//	{
//#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6.0...
//		using namespace std; 
//		pCType_ = &_USE(loc, ctype<E> );
//#else 
//		pCType_ = &std::use_facet<std::ctype<T> >(loc);
//#endif 
//	} 
//
//	// Conversion... 
//	std::basic_string<E, T, A> operator() (const std::wstring& str) const
//	{ 
//		typename std::basic_string<E, T, A>::size_type srcLen =
//			str.length();
//		const wchar_t* pSrcBeg = str.c_str();
//		std::vector<E> tmp(srcLen);
//
//		pCType_->narrow(pSrcBeg, pSrcBeg + srcLen, 'X', &tmp[0]);
//		return std::basic_string<E, T, A>(&tmp[0], srcLen);
//	}
//};

std::string StringConversion::toStdString( const std::wstring & s )
{
	std::string ret;
	ret.resize(s.size(), '\0');
	std::locale loc;
	std::use_facet<std::ctype<wchar_t> >(loc).narrow( s.c_str(), s.c_str()+s.length()+1, '?', &ret[0]);
	return ret;
}

std::wstring StringConversion::toStdWString( const std::string & s )
{
	std::wstring ret;
	ret.resize(s.size(), L'\0');
	std::locale loc;
	std::use_facet<std::ctype<wchar_t> >(loc).widen(s.c_str(), s.c_str()+s.length()+1, &ret[0]);
	return ret;
}

//int test() {
//std::wstring w = L"TEST String@!";
//std::string s = "TEST String@!";
//std::string c = StringConversion::toStdString(w);
//if(c != s) {
//	std::cout << "ERROR" << std::endl;
//	return -1;
//}
//std::wstring c2 = StringConversion::toStdWString(s);
//if(c2 != w) {
//	std::cout << "ERROR" << std::endl;
//	return -1;
//}
//}
