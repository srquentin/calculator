/****************************************************************************
  FileName     [ calcGNum.cpp ]
  PackageName  [ calc ]
  Synopsis     [ Define member functions for class GNum ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <ctype.h>
#include <cassert>
#include "util.h"
#include "calcGNum.h"

// TODO: Initialize the static data members of class GNum
//       (Note: let default _base = 10)
int GNum::_base = 10;
CalcMap GNum::_varMap = CalcMap(); 

// TODO: Define the member functions of class GNum
ostream& operator << (ostream& os, const GNum& n)
{
	os << n._num;   
	return os;
}

GNum GNum::operator + (const GNum& n) const 
{
	GNum temp;
	temp._num = n._num + this->_num; 
	return temp; 
}
GNum& GNum::operator += (const GNum& n) 
{
	_num += n._num; 
	return (*this); 
}
GNum GNum::operator - (const GNum& n) const 
{ 
	GNum temp;
	temp._num = _num - n._num; 
	return 0; 
}
GNum& GNum::operator -= (const GNum& n) 
{
	_num -= n._num; 
	return (*this); 
}
GNum GNum::operator * (const GNum& n) const 
{
	GNum temp;
	temp._num = _num * n._num; 
	return 0; 
}
GNum& GNum::operator *= (const GNum& n) 
{
	_num *= n._num;  
	return (*this); 
}
bool GNum::operator == (const GNum& n) const 
{ 
	return (n == n._num); 
}
bool GNum::operator != (const GNum& n) const 
{ 
	return (n != n._num); 
}
GNum& GNum::operator = (const GNum& n) 
{
	if (this == &n)
		return (*this);
	_num = n._num;
	return (*this); 
}

// [TODO] Set the variable 's' in the _varMap to value 'n',
// no matter the variable 's' exists in _varMap or not
void GNum::setVarVal(const string& s, const GNum& n) 
{ 
	_varMap.insert(CalcMap::value_type(s, n));   
}

// [TODO] Get the value of variable 's'.
// If 's' can be found, store the value in 'n' and return true.
// Otherwise ('s' not found), return false.
bool GNum::getVarVal(const string& s, GNum& n) 
{ 
	CalcMap::iterator search;
	search = _varMap.find(s);
	if (search == _varMap.end())
		return false;	
	else {
		n._num = search->second._num;
		return true; 
	}	
}
// [TODO] If 's' is a valid variable name, return "getVarVal(s, n)";
// else if 's' is a valid number, convert it to GNum and assign to 'n'
bool GNum::getStrVal(const string& s, GNum& n) 
{ 
	int num = 0;
	if (isValidVarName(s))
		return getVarVal(s, n);
	//FIXME: need to revise myStr2Int
	else if (myStr2Int(s, num)) {
		n._num = num;
		return true;
	}
	else
		return false; 

}
//
// [TODO] Print out all the variables in _varMap, one variable per line,
// in the following format (assume _base = 16) ---
// a = #9
// b = #1a
// kkk = #f1c
void GNum::printVars() 
{
	CalcMap::iterator iter;
	char buffer[65];
	for(iter = _varMap.begin(); iter != _varMap.end(); iter++)
	{
			memset(buffer, 0, sizeof(buffer));
			itoa(iter->second._num, buffer, _base);
			cout<<iter->first<<" = #"<<buffer<<endl;
	}
}

void GNum::resetVapMap() 
{ 
	//_varMap.erase(_varMap.begin(), _varMap.end());
	_varMap.clear();
}