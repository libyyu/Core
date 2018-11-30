/*
һ���򵥵Ķ�ά�ı�����з���
*/
#ifndef __FDATATABLE_HPP__
#define __FDATATABLE_HPP__
#include <sstream>
#include <vector>
#include <string>
#include <cassert>
#include "FType.hpp"

_FStdBegin
class FDataTable
{
	typedef std::vector<std::string> ROW_TYPE;
	typedef std::vector<ROW_TYPE> TABLE;
	TABLE _table;
public:
	FDataTable() {}
	FDataTable(FDataTable& ths)
	{
		Empty();
		_table.swap(ths._table);
	}
	FDataTable& operator = (FDataTable& ths)
	{
		Empty();
		_table.swap(ths._table);
		return *this;
	}
	FDataTable(TABLE& tb)
	{
		Empty();
		_table.swap(tb);
	}
	FDataTable& operator = (TABLE& tb)
	{
		Empty();
		_table.swap(tb);
		return *this;
	}
	~FDataTable()
	{
		Empty();
	}

	size_t GetRowCount() const { return _table.size(); }
	size_t GetColCount(size_t row) const { return _table[row].size(); }
	std::string Unit(size_t row, size_t col) const { return _table[row][col]; }

	template<typename T>
	void Update(size_t row, size_t col, T value)
	{
		assert(row < GetRowCount());
		assert(col < GetColCount(row));
		std::stringstream ss;
		ss << value;
		_table[row][col] = ss.str();
	}

	template<typename T>
	bool Convert(const std::string & str, T& value)
	{
		std::stringstream ss;
		ss << str;
		if (!(ss >> value))
			return false;
		return true;
	}

	void Transform(size_t startrow, size_t startcolumn, size_t row_count, size_t col_count, FDataTable & new_table)
	{
		new_table.Empty();
		assert(startrow >= 0 && startrow < GetRowCount());
		assert(startrow + row_count < GetRowCount());
		for (size_t row = startrow; row < startrow + row_count; ++row)
		{
			assert(startcolumn >= 0 && startcolumn < GetColCount(row));
			assert(startcolumn + col_count < GetColCount(row));

			ROW_TYPE & list = new_table.NewRow();
			for (size_t col = startcolumn; col < startcolumn + col_count; ++col)
			{
				list.push_back(Unit(row, col));
			}
		}
	}

	bool GetDataRow(size_t row, size_t startcolumn) { return true; }
	template <typename TYPE1, typename ... ARG_TYPES>
	bool GetDataRow(size_t row, size_t startcolumn, TYPE1 * value, ARG_TYPES*... args)
	{
		if (_table.size() <= row) return false;
		if (_table[row].size() <= startcolumn) return false;

		Convert(_table[row][startcolumn], *value);
		return GetDataRow(row, startcolumn + 1, args...);
	}

	bool GetDataColomn(size_t col, size_t startrow) { return true; }
	template <typename TYPE1, typename ... ARG_TYPES>
	bool GetDataColomn(size_t col, size_t startrow, TYPE1 * value, ARG_TYPES*... args) 
	{ 
		if (_table.size() <= startrow) return false;
		if (_table[startrow].size() <= col) return false;

		Convert(_table[startrow][col], *value);
		return GetDataColomn(startrow+1, col, args...);
	}

	void Empty()
	{
		_table.clear();
	}
	bool IsEmpty() const
	{
		return _table.empty();
	}

	ROW_TYPE& NewRow()
	{
		_table.push_back(ROW_TYPE());
		return _table.back();
	}

	ROW_TYPE& operator [](size_t index)
	{
		assert(index >= 0 && index < GetRowCount());
		return _table[index];
	}

	virtual operator TABLE&()
	{
		return _table;
	}

	std::string ToString(const char* row_delimiter = "\r\n", const char* col_delimiter = ",")
	{
		std::string str = "";
		for (size_t i = 0; i < _table.size(); ++i)
		{
			for (size_t j = 0; j < _table[i].size(); ++j)
			{
				str += EscapeString(_table[i][j]);
				if (j < _table[i].size() - 1)
					str += col_delimiter;
			}
			if (i < _table.size() - 1)
				str += row_delimiter;
		}
		return str;
	}
protected:
	std::string EscapeString(const std::string& str)
	{
		if(str.length()>0)
		{
			return "\"" + str + "\"";
		}
		return str;
	}
private:
};
_FStdEnd
#endif//__FDATATABLE_HPP__

