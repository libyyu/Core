#ifndef __FLOGFILE_HPP__
#define __FLOGFILE_HPP__
#pragma once
#include "FLock.hpp"
#include "FLogInterface.hpp"

_FStdBegin
class FAutoFile
{
public:
    FAutoFile(FILE* fp):_file(fp){}
    FAutoFile(const char* path)
    {
        _file = fopen(path, "w");
        assert(_file && "create log file failed!");
    }
    ~FAutoFile()
    {
        if(_file){
            fflush(_file);
            fclose(_file);
        }
    }
    inline operator FILE*(){ return _file; }
    inline operator FLock*() { return &_lock; }
private:
    FILE* _file;
    FLock _lock;
};
class FLogFile : public FLogInterface
{
public:
    FLogFile(FAutoFile& fp, F_LOGLEVEL level) : FLogInterface(level), _flog(fp)
	{
	}
	FLogFile(FAutoFile& fp, F_LOGLEVEL level, const char* filename, int line = -1)
	:FLogInterface(level, filename, line), _flog(fp)
	{
	}
    virtual ~FLogFile() {}
protected:
    virtual void _LogImpl()
    {
        lock_wrapper lock(_flog);
        FILE* fp = _flog;
        assert(fp && "log file handle is null.");
        fwrite(_message.c_str(), _message.size(), sizeof(char), fp);
    }
protected:
    FAutoFile& _flog;
};

_FStdEnd

//////////////////////////////////////////////////////////////////////
_FStdBegin
#define F_LOGFILE(LEVEL, file) \
	FLogFinisher() = FLogFile(file, F_LOGLEVEL::F_LOGLEVEL_##LEVEL)

#define F_LOGFILE_TRACE(file)  \
	FLogFile f_logfile_trace(file, F_LOGLEVEL::F_LOGLEVEL_TRACE, __FILE__, __LINE__);  \
	FLogTraceFunction f_logfileTraceFunction(f_logfile_trace, __FUNCTION__, __FILE__, __LINE__); \
	f_logfileTraceFunction = f_logfile_trace << __FUNCTION__ << "() enter " << endl;

_FStdEnd

#endif//__FLOGFILE_HPP__