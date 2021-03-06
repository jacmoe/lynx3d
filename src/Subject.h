#pragma once

#include <assert.h>
#include <vector>

template<typename TObservation>
class CObserver
{
public:
    virtual void Notify(TObservation){}
};

template<typename TObservation>
class CSubject
{
public:
    void AddObserver(CObserver<TObservation>* observer)
    {
        assert(observer);
        m_list.push_back(observer);
    }
    void RemoveObserver(CObserver<TObservation>* observer)
    {
        typename std::vector<CObserver<TObservation>*>::iterator it;

        for(it=m_list.begin();it!=m_list.end();it++)
        {
            if(*it==observer)
            {
                m_list.erase(it);
                break;
            }
        }
    }
    void RemoveAllObserver()
    {
        m_list.clear();
    }

    void NotifyAll(TObservation observation)
    {
        typename std::vector<CObserver<TObservation>*>::iterator iter;
        for(iter=m_list.begin();iter!=m_list.end();iter++)
            (*iter)->Notify(observation);
    }

protected:
    std::vector<CObserver<TObservation>*> m_list;
};
