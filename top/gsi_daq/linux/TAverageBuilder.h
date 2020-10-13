///////////////////////////////////////////////////////////////////////////////
// Name:        TAverageBuilder.h
// Purpose:     Template for building of sliding average values.
// Author:      Ulrich Becker (www.INKATRON.de)
// Modified by:
// Created:     2017.06.22
// Copyright:   Sartorius Lab Instruments GmbH & Co. KG, 2017
///////////////////////////////////////////////////////////////////////////////
/*!
 * @file      TAverageBuilder.h
 * @brief     Template for building of sliding average values.
 * @author    Ulrich Becker
 * @copyright Sartorius Lab Instruments GmbH & Co. KG, 2017
 * @date      2017.06.22
 */

#ifndef TAVERAGE_BUILDER__INCLUDED
#define TAVERAGE_BUILDER__INCLUDED
#include <vector>
#include <assert.h>

namespace Scu
{

//! @todo Check possible overflow of attribute "m_summe"!
//!       at the moment this danger isn't given yet if this module
//!       will used for the class "MagnetButton" only.

template <typename T = unsigned int> class TAverageBuilder
{
    std::vector<T>                      m_vector;
    typename std::vector<T>::iterator   m_it;
    T                                   m_summe;

public:

    TAverageBuilder( std::size_t size = 10, const T initVal = static_cast<T>(0) )
    {
        assert( size > 0 );
        m_vector.resize( size );
        init( initVal );
        m_it = m_vector.begin();
    }

    void init( T val )
    {
        m_summe = val * m_vector.size();
        for( auto& item : m_vector )
        {
            item = val;
        }
    }

    T getAverage( void )
    {
        return m_summe / m_vector.size();
    }

    void calculate( T newVal )
    {
        m_summe -= *m_it;
        m_summe += newVal;
        *m_it = newVal;
        m_it++;
        if (m_it == m_vector.end())
        {
            m_it = m_vector.begin();
        }
    }

    TAverageBuilder& operator=( T newVal )
    {
        calculate( newVal );
        return *this;
    }

    T operator()( T newVal )
    {
        calculate( newVal );
        return getAverage();
    }

    T operator()( void )
    {
        return getAverage();
    }

    std::size_t getSize( void ) const
    {
        return m_vector.size();
    }
};

}

#endif // ifndef TAVERAGE_BUILDER__INCLUDED
//================================== EOF ======================================
