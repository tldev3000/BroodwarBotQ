#pragma once
#include <windows.h>
#include "BaseData.h"
#include <iostream>
#include <string>


template< class T>
class Data: public BaseData
{
public:
	Data();
	~Data();

	const T& getValue() const;
	T* beginEdit();
	void endEdit();

	const std::string& getName() const;

	std::ostream& operator <<(const std::ostream& os) const;
	std::istream& operator >>(const std::istream& is) const;

    bool is_sychronized()
    {
        return _synchronized;
    }

    void synchronized()
    {
        _synchronized = true;
    }

private:
    bool _synchronized;
	T value;
//	HANDLE ghMutex;
	std::string data_name;
};
