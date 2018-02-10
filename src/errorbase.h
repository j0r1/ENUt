#ifndef NUT_ERRORBASE_H

#define NUT_ERRORBASE_H

#include "nutconfig.h"
#include <string>

namespace nut
{
	
class ErrorBase
{
public:
	ErrorBase()									{ m_objectName = std::string("Unnamed object"); }
	ErrorBase(const std::string &objName)						{ m_objectName = objName; }
	std::string getObjectName() const						{ return m_objectName; }
	std::string getErrorString() const						{ return m_errorString; }
protected:
	void setErrorString(const std::string &str) const				{ m_errorString = str; }
private:
	mutable std::string m_errorString;
	std::string m_objectName;
};

}

#endif // NUT_ERRORBASE_H

