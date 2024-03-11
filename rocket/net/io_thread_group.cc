#include"./io_thread_group.h"
#include"/home/zxl/rocket/rocket/common/log.h"

namespace rocket{

IOThreadGroup::IOThreadGroup(int size):m_size(size){
    m_io_thread_groups.resize(size);
    for(size_t i=0;i<(size_t)size;i++)
    {
        m_io_thread_groups[i] = new IOThread();
    }
}

IOThreadGroup::~IOThreadGroup()
{
    // m_io_thread_groups.clear();
}

void IOThreadGroup::start()
{
    for(size_t i=0;i<(size_t)m_io_thread_groups.size();i++)
    {
        m_io_thread_groups[i]->start();
    }
}
void IOThreadGroup::join()
{
    for(size_t i=0;i<(size_t)m_io_thread_groups.size();i++)
    {
        m_io_thread_groups[i]->join();
    }
}
IOThread* IOThreadGroup::getIOThread()
{
    if(m_index==(size_t)m_io_thread_groups.size()||m_index==-1)
    {
        m_index=0;
    }
    return m_io_thread_groups[m_index++];
}

}