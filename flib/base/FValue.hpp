#ifndef _FVALUE_HPP__
#define _FVALUE_HPP__
#pragma once
#include "FType.hpp"
#include "FMemory.hpp"
typedef enum _flib_value_type_t {
  /**
   * @const VALUE_TYPE_INVALID
   * 无效类型。
   */
  VALUE_TYPE_INVALID = 0,
  /**
   * @const VALUE_TYPE_BOOL
   * BOOL类型。
   */
  VALUE_TYPE_BOOL,
  /**
   * @const VALUE_TYPE_INT8
   * int8_t类型。
   */
  VALUE_TYPE_INT8,
  /**
   * @const VALUE_TYPE_UINT8
   * uint8_t类型。
   */
  VALUE_TYPE_UINT8,
  /**
   * @const VALUE_TYPE_INT16
   * int16_t类型。
   */
  VALUE_TYPE_INT16,
  /**
   * @const VALUE_TYPE_UINT16
   * uint16_t类型。
   */
  VALUE_TYPE_UINT16,
  /**
   * @const VALUE_TYPE_INT32
   * int32_t类型。
   */
  VALUE_TYPE_INT32,
  /**
   * @const VALUE_TYPE_UINT32
   * uint32_t类型。
   */
  VALUE_TYPE_UINT32,
  /**
   * @const VALUE_TYPE_INT64
   * int64_t类型。
   */
  VALUE_TYPE_INT64,
  /**
   * @const VALUE_TYPE_UINT64
   * uint64_t类型。
   */
  VALUE_TYPE_UINT64,
  /**
   * @const VALUE_TYPE_FLOAT
   * float_t类型。
   */
  VALUE_TYPE_FLOAT,
  /**
   * @const VALUE_TYPE_DOUBLE
   * double类型。
   */
  VALUE_TYPE_DOUBLE,
  /**
   * @const VALUE_TYPE_STRING
   * char*类型。
   */
  VALUE_TYPE_STRING,
  /**
   * @const VALUE_TYPE_WSTRING
   * wchar_t*类型。
   */
  VALUE_TYPE_WSTRING,
  /**
   * @const VALUE_TYPE_POINTER
   * void*类型。
   */
  VALUE_TYPE_POINTER,
  /**
   * @const VALUE_TYPE_OBJECT
   * object_t*类型。
   */
  VALUE_TYPE_OBJECT,
  /**
   * @const VALUE_TYPE_ENUM
   * object_t*类型。
   */
  VALUE_TYPE_ENUM
} flib_value_type_t;

typedef struct _flib_object_t {
  char* name;
  void* object;
  size_t ref_count;
  _flib_object_t(void* o, const char* n):ref_count(0)
  {
        ref_count++;
        object = o;
        char* s = NULL;
        if(n)
        {
            size_t len = strlen(n);
            s = (char*)FLIB_ALLOC(len+1);
            memcpy(s, n, len);
            s[len] = '\0';
        }
        name = s;
  }
  _flib_object_t(_flib_object_t & other):ref_count(0)
  {
        ref_count++;
        other.ref_count++;
        object = other.object;
        if(name) FLIB_FREE((void*)name);
        name = other.name;
  }
  _flib_object_t& operator=(_flib_object_t &other)
  {
        ref_count++;
        other.ref_count++;
        object = other.object;
        if(name) FLIB_FREE((void*)name);
        name = other.name;
        return *this;
  }
  ~_flib_object_t()
  {
      ref_count--;
      if(ref_count <1)
      {
          if(name) FLIB_FREE((void*)name);
          name = NULL;
          
      }
  }
}flib_object_t;

typedef struct _flib_enum_t {
    int32 value;
    char* name;
    size_t ref_count;
    _flib_enum_t(int32 v, const char* n):ref_count(0)
    {
        ref_count++;
        value = v;
        char* s = NULL;
        if(n)
        {
            size_t len = strlen(n);
            s = (char*)FLIB_ALLOC(len+1);
            memcpy(s, n, len);
            s[len] = '\0';
        }
        name = s;
    }
    _flib_enum_t(_flib_enum_t & other):ref_count(0)
    {
        ref_count++;
        other.ref_count++;
        value = other.value;
        if(name) FLIB_FREE((void*)name);
        name = other.name;
    }
    _flib_enum_t& operator=(_flib_enum_t &other)
    {
        ref_count++;
        other.ref_count++;
        value = other.value;
        if(name) FLIB_FREE((void*)name);
        name = other.name;
        return *this;
    }
    ~_flib_enum_t()
    {
        ref_count--;
        if(ref_count <1)
        {
            if(name) FLIB_FREE((void*)name);
            name = NULL;
            
        }
    }
}flib_enum_t;

class FValue
{
    flib_value_type_t type ;
    uchar free_handle ;
    union 
    {
        int8 i8;
        uint8 u8;
        int16 i16;
        uint16 u16;
        int32 i32;
        uint32 u32;
        int64 i64;
        uint64 u64;
        float f;
        double f64;
        bool b;
        const char* str;
        const wchar_t* wstr;
        void* ptr;
        flib_object_t* object;
        flib_enum_t* e;
    } value;
    void reset()
    {
        if (free_handle) 
        {
            switch (type) 
            {
            case VALUE_TYPE_STRING: 
            {
                FLIB_FREE((void*)value.str);
                value.str = NULL;
                break;
            }
            case VALUE_TYPE_WSTRING: 
            {
                FLIB_FREE((void*)value.wstr);
                value.wstr = NULL;
                break;
            }
            case VALUE_TYPE_OBJECT: 
            {
                if(value.object)
                {
                    delete value.object;
                    value.object = NULL;
                }
                break;
            }
            case VALUE_TYPE_ENUM:
            {
                if(value.e)
                {
                    delete value.e;
                    value.e = NULL;
                }
                break;
            }
            default:
                break;
            }
        }
        memset(this, 0x00, sizeof(FValue));
    }
public:
    FValue()
    {
        type = VALUE_TYPE_INVALID;
        free_handle = 0;
        memset(&value, 0x00, sizeof(value));
    }
    ~FValue()
    {
        this->reset();
    }
public:
    FValue* clone()
    {
        FValue* value = new FValue();
        value->copy(*this);
        return value;
    }
    FValue& copy(const FValue& ths)
    {
        reset();
        memcpy(this, &ths, sizeof(FValue));
        free_handle = 0;
        switch (ths.type) 
        {
            case VALUE_TYPE_STRING: 
            {
                char* s = NULL;
                if(ths.value.str)
                {
                    size_t len = strlen(ths.value.str);
                    s = (char*)FLIB_ALLOC(len+1);
                    memcpy(s, ths.value.str, len);
                    s[len] = '\0';
                }
                value.str = s;
                free_handle = value.str != NULL ? 1 : 0;
                break;
            }
            case VALUE_TYPE_WSTRING: 
            {
                size_t size = 0;
                wchar_t* new_str = NULL;
                if(ths.value.wstr)
                {
                    size = wcslen(ths.value.wstr) + 1;
                    new_str = (wchar_t*)FLIB_ALLOC(size * sizeof(wchar_t));
                    memcpy(new_str, ths.value.wstr, size * sizeof(wchar_t));
                    new_str[size] = L'0';
                }
                value.wstr = new_str;
                free_handle = value.wstr != NULL ? 1 : 0;
                break;
            }
            case VALUE_TYPE_OBJECT: 
            {
                value.object = ths.value.object;
                free_handle = value.object != NULL ? 1 : 0;
                break;
            }
            case VALUE_TYPE_ENUM:
            {
                value.e = new flib_enum_t(ths.value.e->value, ths.value.e->name);
                free_handle = value.e != NULL ? 1 : 0;
                break;
            }
            default:
            break;
        }
        return *this;
    }
public:
    template<typename T> void set(T);
    void set(bool b)
    {
        reset();
        value.b = b;
        type = VALUE_TYPE_BOOL;
        free_handle = 0;
    }
    void set(int8 i8)
    {
        reset();
        value.i8 = i8;
        type = VALUE_TYPE_INT8;
        free_handle = 0;
    }
    void set(uint8 u8)
    {
        reset();
        value.u8 = u8;
        type = VALUE_TYPE_UINT8;
        free_handle = 0;
    }
    void set(int16 i16)
    {
        reset();
        value.i16 = i16;
        type = VALUE_TYPE_INT16;
        free_handle = 0;
    }
    void set(uint16 u16)
    {
        reset();
        value.u16 = u16;
        type = VALUE_TYPE_UINT16;
        free_handle = 0;
    }
    void set(int32 i32)
    {
        reset();
        value.i32 = i32;
        type = VALUE_TYPE_INT32;
        free_handle = 0;
    }
    void set(uint32 u32)
    {
        reset();
        value.u32 = u32;
        type = VALUE_TYPE_UINT32;
        free_handle = 0;
    }
    void set(int64 i64)
    {
        reset();
        value.i64 = i64;
        type = VALUE_TYPE_INT64;
        free_handle = 0;
    }
    void set(uint64 u64)
    {
        reset();
        value.u64 = u64;
        type = VALUE_TYPE_UINT64;
        free_handle = 0;
    }
    void set(float f)
    {
        reset();
        value.f = f;
        type = VALUE_TYPE_FLOAT;
        free_handle = 0;
    }
    void set(double d)
    {
        reset();
        value.f64 = d;
        type = VALUE_TYPE_DOUBLE;
        free_handle = 0;
    }
    void set(const char* str)
    {
        reset();
        if(value.str) FLIB_FREE((void*)value.str);
        char* s = NULL;
        if(str)
        {
            size_t len = strlen(str);
            s = (char*)FLIB_ALLOC(len+1);
            memcpy(s, str, len);
            s[len] = '\0';
        }
        value.str = s;
        free_handle = value.str != NULL ? 1 : 0;
        type = VALUE_TYPE_STRING;
    }
    void set(const wchar_t* wstr)
    {
        reset();
        size_t size = 0;
        wchar_t* new_str = NULL;
        if(wstr)
        {
            size = wcslen(wstr) + 1;
            new_str = (wchar_t*)FLIB_ALLOC(size * sizeof(wchar_t));
            memcpy(new_str, wstr, size * sizeof(wchar_t));
            new_str[size] = L'0';
        }
        value.wstr = new_str;
        free_handle = value.wstr != NULL ? 1 : 0;
        type = VALUE_TYPE_WSTRING;
    }
    void set(void* ptr)
    {
        reset();
        value.ptr = ptr;
        type = VALUE_TYPE_POINTER;
        free_handle = 0;
    }
    void set(flib_object_t* o)
    {
        reset();
        value.object = o;
        type = VALUE_TYPE_OBJECT;
        free_handle = 0;
    }
    void set(flib_enum_t* e)
    {
        reset();
        value.e = new flib_enum_t(e->value, e->name);
        type = VALUE_TYPE_ENUM;
        free_handle = 1;
    }
public:
    flib_value_type_t get_type() const
    {
        return type;
    }
    bool get_bool() const
    {
        assert(type == VALUE_TYPE_BOOL);
        return value.b;
    }
    int8 get_int8() const
    {
        assert(type == VALUE_TYPE_INT8);
        return value.i8;
    }
    uint8 get_uint8() const
    {
        assert(type == VALUE_TYPE_UINT8);
        return value.u8;
    }
    int16 get_int16() const
    {
        assert(type == VALUE_TYPE_INT16);
        return value.i16;
    }
    uint16 get_uint16() const
    {
        assert(type == VALUE_TYPE_UINT16);
        return value.u16;
    }
    int32 get_int32() const
    {
        assert(type == VALUE_TYPE_INT32);
        return value.i32;
    }
    uint32 get_uint32() const
    {
        assert(type == VALUE_TYPE_UINT32);
        return value.u32;
    }
    int64 get_int64() const
    {
        assert(type == VALUE_TYPE_INT64);
        return value.i64;
    }
    uint64 get_uint64() const
    {
        assert(type == VALUE_TYPE_UINT64);
        return value.u64;
    }
    float get_float() const
    {
        assert(type == VALUE_TYPE_FLOAT);
        return value.f;
    }
    double get_double() const
    {
        assert(type == VALUE_TYPE_DOUBLE);
        return value.f64;
    }
    const char* get_string() const
    {
        assert(type == VALUE_TYPE_STRING);
        return value.str;
    }
    const wchar_t* get_wstring() const
    {
        assert(type == VALUE_TYPE_WSTRING);
        return value.wstr;
    }
    void* get_pointer() const
    {
        assert(type == VALUE_TYPE_POINTER);
        return value.ptr;
    }
    flib_object_t* get_object() const
    {
        assert(type == VALUE_TYPE_OBJECT);
        return value.object;
    }
    flib_enum_t* get_enum() const
    {
        assert(type == VALUE_TYPE_ENUM);
        return value.e;
    }
};

#endif//_FVALUE_HPP__