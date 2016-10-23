#include <string>
#include <vector>
#include <boost/format.hpp>
#include <system_error>
#include "utils.h"

using std::string;
using std::vector;
using boost::format;
using std::system_error;
using boost::str;
using std::system_category;

const vector<string> SyscallNames = {
#include "syscallent.h"
};

const vector<string> SignalNames = {
#include "signalent.h"
};

string SyscallToString(int syscall)
{
    if (syscall < SyscallNames.size())
    {
        return SyscallNames[syscall];
    }
    else
    {
        return str(format("unknown syscall %1%") % syscall); 
    }
}

string SignalToString(int signal)
{
    if (signal < SignalNames.size())
    {
        return SignalNames[signal];
    }
    else
    {
        return str(format("unknown signal %1%") % signal); 
    }
}

void Ensure_Seccomp(int XX)
{
    if (XX != 0)
    {
        throw system_error(XX < 0 ? -XX : XX, system_category());
    }
}

int Ensure(int XX)
{
    return EnsureNot(XX, -1);
}

void Ensure0(int XX)
{
    if (XX != 0)
    {
        int err = errno;
        throw system_error(err, system_category());
    }
}
