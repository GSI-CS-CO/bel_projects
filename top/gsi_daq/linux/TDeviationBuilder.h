///////////////////////////////////////////////////////////////////////////////
// Name:        TDeviationBuilder.h
// Purpose:     Calculates maximum absolute deviation of a row of values.
// Author:      Ulrich Becker (www.INKATRON.de)
// Modified by:
// Created:     2017.08.15
// Copyright:   Sartorius Lab Instruments GmbH & Co. KG, 2017
///////////////////////////////////////////////////////////////////////////////
/*!
 * @file      TDeviationBuilder.h
 * @brief     Template for calculating the maximum absolute deviation of a row
 *            of values.
 * @author    Ulrich Becker
 * @copyright Sartorius Lab Instruments GmbH & Co. KG, 2017
 * @date      2017.08.15
 *
 *
 * Example:
 * @code
 *  .
 *  .
 * 1003
 * 1002 --------------*-----------------+-> maximum
 * 1001      *       *                  |
 * 1000  **** ***  **   ****   ****     +--> maximum deviation: 1002 - 998 = 4
 *  999          **         * *         |
 *  998 ---------------------*----------+-> minimum
 *  997
 *   .
 *   .
 * @endcode
 */
#ifndef _TDEVIATIONBUILDER_H
#define _TDEVIATIONBUILDER_H

#include <vector>
#include <assert.h>

namespace Scu
{

//=============================================================================

template <typename T = unsigned int> class TDeviationBuilder
{
    std::vector<T>                      m_vector;
    typename std::vector<T>::iterator   m_it;
    T                                   m_maxDeviation;
    T                                   m_max;
    T                                   m_min;
    T                                   m_average;
    bool                                m_isReady;

public:
    TDeviationBuilder( const std::size_t size = 10 )
    {
        assert( size >= 2 );
        m_vector.resize( size );
        reset();
    }

    void reset( void )
    {
        m_maxDeviation = static_cast<T>(0);
        m_max          = static_cast<T>(0);
        m_min          = static_cast<T>(0);
        m_average      = static_cast<T>(0);
        m_isReady = false;
        m_it = m_vector.begin();
    }

    std::size_t getSize( void ) const
    {
        return m_vector.size();
    }

    bool isReady( void ) const
    {
        return m_isReady;
    }

    T getDeviation( void ) const
    {
        return m_maxDeviation;
    }

    T operator()( void ) const
    {
        return getDeviation();
    }

    T getMinimum( void ) const
    {
        return m_min;
    }

    T getMaximum( void ) const
    {
        return m_max;
    }

    bool add( T value )
    {
        *m_it = value;
        if (m_it < m_vector.end() - 1)
        {
            m_it++;
        }
        else
        {
            m_it = m_vector.begin();
            m_isReady = true;
        }

        if ( m_isReady )
        {
            calculate();
            return true;
        }
        return false;
    }

    void operator+=( T value )
    {
        add( value );
    }

    void operator=( T value )
    {
        reset();
        add( value );
    }

    T getAverage( void ) const
    {
        return m_average;
    }

private:

    void calculate( void )
    {
        m_max = *m_vector.begin();
        m_min = m_max;
        m_average = m_max;
        for (auto i = m_vector.begin() + 1; i < m_vector.end(); i++)
        {
            m_average += *i;
            if (*i < m_min)
            {
                m_min = *i;
            }
            else if (*i > m_max)
            {
                m_max = *i;
            }
        }
        m_maxDeviation = m_max - m_min;
        m_average /= m_vector.size();
    }
};

}

#endif // ifndef _TDEVIATIONBUILDER_H
//================================== EOF ======================================
