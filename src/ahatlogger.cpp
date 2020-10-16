#include "ahatlogger.h"
#include <algorithm>

std::shared_ptr< std::queue< std::pair<std::string, std::string> > > AhatLogger::q = NULL;
bool			AhatLogger::isStarted = false;
bool			AhatLogger::isFinished = false;
std::string		AhatLogger::path;
std::string		AhatLogger::name;
std::mutex		AhatLogger::mutex;
int				AhatLogger::level;
std::string		AhatLogger::curPath;
std::ofstream	AhatLogger::f;


std::string code(std::string file, std::string func, int line)
{
	std::stringstream buf;
	buf<<file<<","<<func<<":"<<line;
	return buf.str();
}

int AhatLogger::makeDirectory(const char *path)
{
	char tmp_path[2048];
	const char *tmp = path;
	int  len  = 0;
	int  ret;

	while((tmp = strchr(tmp, '/')) != NULL) 
	{
		len = tmp - path;
		tmp++;
		if(len == 0) 
		{
			continue;
		}

		strncpy(tmp_path, path, len);
		tmp_path[len] = 0x00;
		if (existDirectory(tmp_path))
		{
			continue;
		}

#ifdef _WIN32
		size_t tn;
		wchar_t* pwstr = new wchar_t(sizeof(wchar_t) * (len + 1));
		mbstowcs_s(&tn, pwstr, len + 1, tmp_path, len + 1);

		if ((ret = _wmkdir(pwstr)) == -1)
		{
			delete pwstr;
			return -1;
		}
		delete pwstr;
#elif __linux__
		if ((ret = mkdir(tmp_path, S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH)) == -1)
		{
			return -1;
		}
#endif
	}

#ifdef _WIN32
	size_t tn;
	wchar_t* pwstr = new wchar_t(sizeof(wchar_t) * (len + 1));
	mbstowcs_s(&tn, pwstr, len + 1, tmp_path, len + 1);

	ret = _wmkdir(pwstr);
	delete pwstr;

	return ret;
#elif __linux__
	return mkdir(path, S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH);
#endif
}

bool AhatLogger::existDirectory(const char *path)
{
	if ( path == NULL) 
		return false;

	bool bExists = false;
#ifdef _WIN32
	FILE *f = NULL;
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		bExists = true;
	}
	else
	{
		fclose(f);
		bExists = false;
	}

#elif __linux__
	DIR *pDir;
	pDir = opendir (path);

	if (pDir != NULL)
	{
		bExists = true;    
		(void) closedir (pDir);
	}
#endif
	return bExists;
}

std::string AhatLogger::getDate()
{
	std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
	
	time_t tt = std::chrono::system_clock::to_time_t(time);
	struct tm tm;
	localtime_s(&tm, &tt);
 
	std::stringstream s;
	s << tm.tm_year+1900 << "-" << std::setfill('0') << std::setw(2) << tm.tm_mon+1 << "-" << 
		std::setfill('0') << std::setw(2) << tm.tm_mday;

	return s.str();
}

void AhatLogger::start()
{
	curPath = "";
	if(isStarted == false)
	{
		isStarted = true;
		logOpen();

		q = std::make_shared<std::queue< std::pair<std::string, std::string> > >();
		std::thread t(&AhatLogger::run);
		t.detach();
	}
}

bool AhatLogger::logOpen()
{
	std::string date = getDate();
	std::string filepath = "";

	filepath += path;
	if (filepath != "")
		filepath += "/";
	filepath += name;
	filepath += date;
	filepath += ".log";
	curPath = filepath;

	f.open(filepath, std::ios::out | std::ios::app);
	if (f.fail())
	{
		return false;
	}

	return true;
}

bool AhatLogger::logClose()
{
	f.close();

	return true;
}

void AhatLogger::stop()
{
	isStarted = false;
	
	while(1)
	{
		if(isFinished)
			break;
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	isFinished = false;
}

void AhatLogger::run()
{
	while(isStarted)
	{
		if(!q->size())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		else
		{
			if (!logWrite())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
	if(q->size())
	{
		logWrite();
	}
	
	isFinished = true;
}

bool AhatLogger::logWrite()
{
	std::string date = getDate();
	std::string filepath = "";
	
	filepath += path;
	if(filepath != "")
		filepath += "/";
	filepath += name;
	filepath += date;
	filepath += ".log";

	if (curPath.compare(filepath) != 0)
	{
		logClose();
		logOpen();
	}
	
	mutex.lock();

	auto qq = q;
	q = std::make_shared<std::queue< std::pair<std::string, std::string> > >();

	mutex.unlock();

	int size = qq->size();
	for(int i = 0; i < size; i++)
	{
		std::pair<std::string, std::string>  item = qq->front();
		qq->pop();

		f << item.first << "," << getCurTime() << item.second << "\n";
	}
	f.flush();
		
	return true;
}

void AhatLogger::setting(std::string path, std::string filename, int level)
{
	if (path.empty())
	{
#ifdef _WIN32
	#ifdef __MINGW32__
		char tmp[256];
		int len = GetModuleFileName(NULL, tmp, MAX_PATH);
		std::string buf = tmp;
		buf = buf.substr(0, buf.find_last_of("\\"));
	#else
		/*
		wchar_t tmp[256];
		int len = GetModuleFileName(NULL, tmp, MAX_PATH);
		std::wstring ws(tmp);
		buf = buf.substr(0, buf.find_last_of("\\"));
		*/
		std::string buf(".");
	#endif
#elif __linux__
		char buf[256];
		int len = readlink("/proc/self/exe", buf, 256);
		buf[len] = '\0';
#endif
		AhatLogger::path = buf;
	}
	else
	{
		AhatLogger::path = path;
	}

	std::replace( AhatLogger::path.begin(), AhatLogger::path.end(), '\\', '/');

	if (!existDirectory(path.c_str()))
	{
		if (makeDirectory(path.c_str()) == -1)
		{
			AhatLogger::path = "";
		}
	}

	AhatLogger::name = filename;
	AhatLogger::name += "_";
	AhatLogger::level = level;
}

void AhatLogger::INFO(std::string src_file, const char* body, ...)
{
    int count = 0;
 
	va_list arglist;
	va_start(arglist, body);

	int len = _vscprintf(body, arglist) + 1;
	char* tmp = new char[len];
	memset(tmp, 0, len);

	vsprintf_s(tmp, len, body, arglist);
	
	AhatLogItemInfo log(src_file, tmp);
	delete []tmp;
	va_end(arglist);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("INFO", log.message()));
	mutex.unlock();
}

void AhatLogger::ERR(std::string src_file, const char* body, ...)
{
    int count = 0;
 
	va_list arglist;
	va_start(arglist, body);

	int len = _vscprintf(body, arglist) + 1;
	char* tmp = new char[len];
	memset(tmp, 0, len);

	vsprintf_s(tmp, len, body, arglist);
	
	AhatLogItemError log(src_file, tmp);
	delete []tmp;
	va_end(arglist);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("ERROR", log.message()));
	mutex.unlock();
}

void AhatLogger::CUSTOM(std::string src_file, std::string custom, const char* body, ...)
{
    int count = 0;
 
	va_list arglist;
	va_start(arglist, body);

	int len = _vscprintf(body, arglist) + 1;
	char* tmp = new char[len];
	memset(tmp, 0, len);

	vsprintf_s(tmp, len, body, arglist);
	
	AhatLogItemCustom log(src_file, tmp);
	delete []tmp;
	va_end(arglist);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>(custom, log.message()));
	mutex.unlock();
}

//로그레벨 0이면 로그를 작성 그 이상은 작성하지 않음
void AhatLogger::DEBUG(std::string src_file, const char* body, ...)
{
	if(level != 0)
		return;
	
    int count = 0;
 
	va_list arglist;
	va_start(arglist, body);

	int len = _vscprintf(body, arglist) + 1;
	char* tmp = new char[len];
	memset(tmp, 0, len);

	vsprintf_s(tmp, len, body, arglist);
	
	AhatLogItemDebug log(src_file, tmp);
	delete []tmp;
	va_end(arglist);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("DEBUG", log.message()));
	mutex.unlock();
}

void AhatLogger::REQUEST(std::string src_file, std::string req_ip, std::string req_uri)
{
	AhatLogItemRequest log(src_file, req_ip, req_uri, src_file);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("REQUEST", log.message()));
	mutex.unlock();
}
void AhatLogger::RESPONSE(std::string src_file, std::string res_body, std::string res_code)
{
	AhatLogItemResponse log(src_file, res_body, res_code);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("RESPONSE", log.message()));
	mutex.unlock();
}

void AhatLogger::DB(std::string src_file, InDBtem db_req_item, std::string db_res_body)
{
	AhatLogItemDB log(src_file, db_req_item, db_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("DB", log.message()));
	mutex.unlock();
}
void AhatLogger::DB_ERR(std::string src_file, InDBtem db_req_item, std::string db_res_body)
{
	AhatLogItemDB log(src_file, db_req_item, db_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("DB_ERROR", log.message()));
	mutex.unlock();
}

void AhatLogger::DB_DEBUG(std::string src_file, InDBtem db_req_item, std::string db_res_body)
{
	if(level != 0)
		return;
	
	AhatLogItemDB log(src_file, db_req_item, db_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("DB_DEBUG", log.message()));
	mutex.unlock();
}
void AhatLogger::DB_ERR_DEBUG(std::string src_file, InDBtem db_req_item, std::string db_res_body)
{
	if(level != 0)
		return;
	
	AhatLogItemDB log(src_file, db_req_item, db_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("DB_ERROR_DEBUG", log.message()));
	mutex.unlock();
}

void AhatLogger::IN_REQ(std::string src_file, InReqItem in_req_item, std::string in_res_body)
{	
	AhatLogItemInReq log(src_file, in_req_item, in_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("IN_REQ", log.message()));
	mutex.unlock();
}

void AhatLogger::IN_REQ_ERR(std::string src_file, InReqItem in_req_item, std::string in_res_body)
{
	AhatLogItemInReq log(src_file, in_req_item, in_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("IN_REQ_ERR", log.message()));
	mutex.unlock();
}

void AhatLogger::IN_REQ_DEBUG(std::string src_file, InReqItem in_req_item, std::string in_res_body)
{
	if(level != 0)
		return;
	
	AhatLogItemInReq log(src_file, in_req_item, in_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("IN_REQ_DEBUG", log.message()));
	mutex.unlock();
}

void AhatLogger::IN_REQ_ERR_DEBUG(std::string src_file, InReqItem in_req_item, std::string in_res_body)
{
	if(level != 0)
		return;
	
	AhatLogItemInReq log(src_file, in_req_item, in_res_body);
	
	mutex.lock();
	AhatLogger::q->push( std::pair<std::string, std::string>("IN_REQ_ERR_DEBUG", log.message()));
	mutex.unlock();
}

#ifdef __linux__
int _vscprintf (const char * format, va_list pargs) 
{ 
    int retval; 
    va_list argcopy; 
    va_copy(argcopy, pargs); 
	
    retval = vsnprintf(NULL, 0, format, argcopy); 
    va_end(argcopy); 
    return retval; 
}
#endif