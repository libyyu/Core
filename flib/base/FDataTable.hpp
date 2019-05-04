/*
一个简单的二维文本表格切分类
*/
#ifndef __FDATATABLE_HPP__
#define __FDATATABLE_HPP__
#include "FType.hpp"
#include <sstream>

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

	bool LoadFromFile(const std::string& file, bool u8Bom=true)
	{
		FILE* fp = fopen(file.c_str(), "r");
		if (!fp) return false;
		long dwPos = ftell(fp);
		fseek(fp, 0, SEEK_END);
		long dwEndPos = ftell(fp);
		long len = dwEndPos - dwPos;
		fseek(fp, dwPos, SEEK_SET);

		if (u8Bom && len >=3)
		{
			const unsigned char bomhead[] = { 0xEF, 0xBB, 0xBF };
			unsigned char bom[3] = { 0 };
			fread(bom, 1, sizeof(bom), fp);
			bool isBom = true;
			for (int i = 0; i < 3; ++i) {
				if (bom[i] != bomhead[i]) {
					isBom = false;
					break;
				}
			}

			if (!isBom) {
				fseek(fp, dwPos, SEEK_SET);
			}
			else {
				len -= 3;
			}
		}
		
		char* data = new char[len];
		memset(data, 0, len);
		size_t nread = fread(data, 1, len, fp);
		fclose(fp);

		data[nread] = 0x0;
		std::string u8(data);
		delete[] data;
		data = NULL;
		
		return LoadFromData(u8);
	}

	bool LoadFromData(const std::string& data)
	{
		Empty();
		if (data.empty()) return false;
		size_t pos = 0;
		std::string grid;
		bool quotation_begin = false;
		bool quotation_end = true;
		ROW_TYPE* pRow = NULL;
		while(pos < data.length())
		{
			if(!pRow)
			{
				ROW_TYPE& row = NewRow();
				pRow = &row;
			}

			if(data[pos] == '\n' && quotation_end)
			{
				if(!grid.empty())
				{
					if(grid.back()=='\r')
					{
						grid.erase(grid.size()-1);
					}
					pRow->push_back(grid);
					grid = "";
					quotation_begin = false;
				}

				ROW_TYPE& row = NewRow();
				pRow = &row;
			}
			else if(data[pos] == '\"')
			{
				if(!quotation_begin) { quotation_begin = true; quotation_end = false; }
				else 
				{
					quotation_end = true;
				}
			}
			else if(data[pos] == ',' && quotation_end)
			{
				pRow->push_back(grid);
				grid = "";
				quotation_begin = false;
			}
			else
			{
				grid += data[pos];
			}
			pos++;
		}
		return true;
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
		for (size_t i = 0; i < _table.size(); i++)
		{
			ROW_TYPE & list = new_table.NewRow();
			size_t col_end = startcolumn + col_count;
			if (col_end > _table[i].size() || col_count == 0) col_end = _table[i].size();
			list.insert(list.begin(), col_end, std::string());
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

	ROW_TYPE& GetRow(size_t row)
	{
		assert(0 <= row && row < GetRowCount());
		return _table[row];
	}

	const ROW_TYPE& GetRow(size_t row) const
	{
		assert(0 <= row && row < GetRowCount());
		return _table[row];
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
	std::string ToString(const ROW_TYPE& row, const char* col_delimiter = ",")
	{
		std::string str = "";
		for (size_t i = 0; i < row.size(); ++i)
		{
			str += EscapeString(row[i]);
			if (i < row.size() - 1)
				str += col_delimiter;
		}
		return str;
	}
protected:
	std::string EscapeString(const std::string& str)
	{
		if (str.find(',') != std::string::npos 
			|| str.find(':') != std::string::npos
			|| str.find('\n') != std::string::npos)
		{
			return "\"" + str + "\"";
		}
		return str;
	}
	std::vector<std::string> Split(const std::string &str, const std::string &pattern)
	{
		std::vector<std::string> resVec;
		if ("" == str) return resVec;

		std::string strs = str + pattern;
		size_t pos = strs.find(pattern);
		size_t size = strs.size();
		while (pos != std::string::npos)
		{
			std::string x = strs.substr(0, pos);
			resVec.push_back(x);
			strs = strs.substr(pos + 1, size);
			pos = strs.find(pattern);
		}

		return resVec;
	}
private:
};
_FStdEnd
#endif//__FDATATABLE_HPP__

